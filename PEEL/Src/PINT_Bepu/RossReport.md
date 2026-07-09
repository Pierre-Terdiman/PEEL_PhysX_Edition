# BepuPhysics2 / AbominationInterop — findings from the PEEL integration

These findings come out of integrating bepuphysics2 into PEEL (a comparative physics-engine test framework) through AbominationInterop, compiled with NativeAOT to a C API. The resulting plugin exercises a fairly broad surface: rigid bodies, compounds, most joint types, contact callbacks, kinematics, and the full query set (rays, sweeps, overlaps, convex casts). The list below contains one engine solver bug, one engine behavior question, four interop bugs, plus toolchain notes and a couple of proposals. Everything was found against the 2.5.0-beta.22 source drop and the current scratchpad AbominationInterop; some local copies carry fixes (marked "PEEL local fix"), so quoted line numbers are approximate for stock files. Repros and diffs are available for all items.

---

## Engine

### 1. `AngularAxisGearMotor.Solve` applies the accumulated impulse instead of the corrective impulse

**File:** `BepuPhysics/Constraints/AngularAxisGearMotor.cs`, end of `Solve`.

```csharp
var csi = (negatedCSVB - unscaledCSVA) * effectiveMass - accumulatedImpulses * softnessImpulseScale;
ServoSettingsWide.ClampImpulse(maximumImpulse, ref accumulatedImpulses, ref csi);
ApplyImpulse(impulseToVelocityA, negatedImpulseToVelocityB, accumulatedImpulses, ref wsvA.Angular, ref wsvB.Angular);
//                                                          ^^^^^^^^^^^^^^^^^^^ should be csi
```

`Solve` re-applies the entire accumulated impulse history every solver iteration instead of the corrective increment `csi`. The sibling `AngularAxisMotor.Solve` passes `csi` at the same spot, which is correct. The result is that the gear constraint never converges and pumps energy: in a PEEL test (two gear-linked wheels, ratio −1, one driven at 4 rad/s) the stock constraint drifts to −5.11/+6.45 rad/s and keeps climbing; with the one-word fix it converges to exactly −4.00/+4.00.

This is the only engine-code change carried locally in the PEEL snapshot — one word, and the constraint goes from broken to textbook.

### 2. Question: convex-manifold friction is budgeted at μ/contactCount

**File:** `BepuPhysics/Constraints/Contact/ContactConvexTypes.tt`, line ~265 (and the generated `ContactConvexTypes.cs`):

```csharp
var premultipliedFrictionCoefficient = new Vector<float>(1f / <#=contactCount#>f) * prestep.MaterialProperties.FrictionCoefficient;
var maximumTangentImpulse = premultipliedFrictionCoefficient * (accumulatedImpulses.Penetration0 + ... + PenetrationN);
```

Since Σ Pᵢ equals the total normal impulse, the whole-manifold tangent budget comes out to μ·N/contactCount rather than μ·N. A box resting on a face (4-contact manifold) gets a quarter of the Coulomb friction that PhysX/Jolt apply for the same μ — measured: a sliding box at μ=1 decelerates at 2.74 m/s² instead of ~9.81. The nonconvex manifolds budget per-contact (μ·Pᵢ each), so mesh and compound contacts follow the usual convention; only the convex one-bundle manifolds diverge.

The explicit premultipliers look deliberate, hence the question: is μ meant to be read as "already averaged over the manifold"? If it's by design it would be worth documenting, because anyone porting content across engines will find convex friction 2–4× weaker at the same coefficient. (The PEEL plugin compensates by scaling μ by the manifold contact count in the narrow-phase material callback — exact regardless of load distribution, precisely because Σ Pᵢ = N.)

---

## Interop (AbominationInterop)

### 3. `InstanceDirectory.Remove` validates the version against the wrong slot

Stock code:

```csharp
if (handle.Version != instances[handle.TypeIndex].Version)
    throw new ArgumentException("Handle is out of date. ...");
```

That indexes by `handle.TypeIndex` instead of `handle.Index`. Since TypeIndex is 0–7 it's always in bounds, so nothing crashes — it just compares against an unrelated slot's version and throws a spurious "Handle is out of date" (in PEEL: on every `DestroySimulation`). Fix: `instances[handle.Index].Version`.

### 4. Handle version wraparound corrupts handles after 16 reuses of a slot

`InstanceHandle` packs the version into 4 bits (bits 24–27, `Version => (RawValue >> 24) & 0xF`), but `InstanceDirectory.Add` increments the slot version unboundedly and the constructor packs `version << 24` unmasked. On the 16th reuse of a slot the stored version (16) no longer matches what the handle can represent (0), so every handle from that point on fails validation — and the overflow bit bleeds into the TypeIndex field (bit 28), so the handle also reads back as the wrong type. In PEEL this fired on the 16th scene create/destroy cycle. Fix: wrap in `Add` (`slot.Version = (slot.Version + 1) & 0xF;`) and assert in the `InstanceHandle` constructor that the version fits in 4 bits.

### 5. `InstanceDirectory.this[int]` has an inverted bounds check

```csharp
public T? this[int index]
{
    get
    {
        if (index < 0 || index > instances.Length)
            return instances[index].Instance;   // dereferences exactly when out of bounds
        return null;                             // returns null for every valid index
    }
}
```

The condition is inverted (and off by one). Nothing calls it today, so it's harmless until something does. Related nit: `Remove`'s index guard uses `handle.Index > instances.Length` where `>=` is meant, so `Index == Length` passes the guard and throws `IndexOutOfRangeException` instead of the intended `ArgumentOutOfRangeException`.

### 6. The SIMD velocity-integration callbacks can't be marshaled under NativeAOT

`PoseIntegratorCallbacksInterop.IntegrateVelocitySIMD128/256` are declared as unmanaged function pointers taking `Vector128<int>`/`Vector256<float>` etc. **by value**. NativeAOT refuses by-value SIMD types in unmanaged signatures — invoking the wide path throws `MarshalDirectiveException` at runtime. Since the C API only exists as a NativeAOT binary, the vectorized callback path is unusable as shipped; only the scalar callback works.

Suggestion: pass the vector parameters by pointer (most of the signature already is), or document the wide path as unavailable. The PEEL build works around it with a managed-side "builtin" gravity/damping integrator (mirroring the demos' callbacks) behind a flag placed in the interop struct's padding byte — which also turned out to be the right configuration for benchmarking the engine itself, since it matches what a native C# user gets after inlining. That diff is available too; measured overhead of the scalar native callback vs builtin is ~50ns/body/frame.

### 7. Wide-callback dispatch silently truncates when `Vector<T>` is 512 bits

The wide dispatch in `PoseIntegratorCallbacks.IntegrateVelocity` used `AsVector128()`/`AsVector256()` on the `Vector<T>` arguments. Building the interop with the .NET 10 ILC and `avx512,vectort512` (16-wide bundles) makes those reinterprets silently take the **lower 8 lanes** of every 16-lane bundle — half the bodies in each bundle never receive velocity integration, with no error anywhere. There's also no 512-bit callback slot in `PoseIntegratorCallbacksInterop`, so there's no correct target to dispatch to.

The local fix changes the dispatch to pointer reinterprets and makes the 16-wide case throw `NotSupportedException` for native SIMD callbacks (scalar callbacks work at any width). Side benefit: the ILC 9 compiler ICEs on those `AsVector128/256` intrinsics in unoptimized 512-bit builds even when the branch is provably dead — pointer reinterprets dodge that as well.

---

## Toolchain notes (Microsoft's bugs, not AbominationInterop's — flagged because they bite this API's users)

- **ILC 8.0.12:** a static field of a `System.Numerics` SIMD type (e.g. `Vector3`) reachable from an `UnmanagedCallersOnly` export crashes the compiler (`InvalidCastException` in `recordRelocation`). Workaround: store scalar floats.
- **ILC 9 (9.0.1 and 9.0.17):** enabling `vectort512` miscompiles — `Vector<float>.Count` reports 8 but the generated code is width-inconsistent, crashing at runtime in `PoseIntegrator.PredictBoundingBoxes`. The same IL runs correctly 16-wide under the .NET 9 JIT, so it's AOT-specific. Fixed in the .NET 10 ILC, which is the first AOT compiler where 512-bit `Vector<T>` actually works. A standalone repro exists and will be filed with dotnet/runtime.

---

## Proposals / gaps

### 8. `AngularAxisGearMotor` can't express perpendicular-axis (bevel) gears

The constraint shares a single axis anchored to body A, so bevel/differential gear trains can't be built. A per-body-axis variant (`LocalAxisA`/`LocalAxisB`) was prototyped and validated on a bevel pair (Z-axis wheel driving an X-axis wheel, ratio −1, exact ±4 rad/s convergence), then reverted locally to stay stock. The diff is available on request — with one important implementation note: the type processor's body-B gather filter must change from `AccessOnlyAngularWithoutPose` to `AccessOnlyAngular`, because the constraint then needs `orientationB`; with the stock filter, orientationB is never gathered and the garbage propagates to NaN poses and nondeterministic broadphase crashes. A new constraint type may be preferable to changing the existing description layout.

### 9. No angular↔linear coupling constraint

There's nothing in `Constraints/` that couples angular velocity to linear velocity, so rack-and-pinion (or screw) joints can't be assembled from existing pieces. A `RackMotor` analogous to `AngularAxisGearMotor` looks like ~150 lines, if there's upstream interest.

---

## Observations (no action requested — possibly interesting data)

- **True 16-wide `Vector<T>` is a pessimization for the engine.** Pyramid stack, 3240 boxes, 8 threads, same compiler (.NET 10 ILC) for all variants: SSE2 ~2.4 ms, AVX2 ~1.7–1.9 ms, AVX-512 16-wide ~2.0–2.15 ms — and AVX-512 with 8-wide bundles (EVEX encoding only) is the fastest of all at ~1.6 ms. The hand-tuned intrinsic paths (`GatherState`, `TransposeMotionStates`, `ScatterPose`, the `BundleIndexing` fast paths) are `Count == 8`-gated and fall back to scalar loops at 16-wide, and small batches waste lanes.
- **Dzhanibekov effect:** the implicit gyroscopic integrator dissipates the intermediate-axis instability — a tumbling T-handle settles onto its max-inertia axis within seconds at 1/60 steps (zero flips over 20 s). `AllowSubstepsForUnconstrainedBodies` + 8–16 substeps restores qualitative flips, though |ω| still decays (~9.0 → 7.2 over 20 s at zero damping). Related surprise: without that flag, free bodies integrate once per frame regardless of the substep count.

---

Beyond this list, the integration was remarkably smooth, and the engine holds up very well in the comparative tests.
