
	class VoidPlug : public IcePlugin
	{
		public:

		virtual	const char*		GetPlugName()			{ return null;	}
		virtual	const char*		GetMasterAPI()			{ return null;	}
		virtual	const char*		GetPlugDescription()	{ return null;	}
		virtual	udword			GetNbInterfaces()		{ return 0;		}
		virtual	IceInterface*	GetInterface(udword i)	{ return null;	}

		virtual	bool			Init()					{ return true;	}
		virtual	bool			Close()					{ return true;	}
	};

static VoidPlug ThePlug;

FUNCTION __declspec(dllexport) udword GetSDKVersion()	{ return FLEXINE_SDK_VERSION;	}
FUNCTION __declspec(dllexport) IcePlugin* GetPlugin()	{ return &ThePlug;				}

#ifdef ICE_MAIN

bool TestCallback(RMScene* scene, udword a, udword b);

void ModuleAttach(udword hmod)
{
	RMScene* TheScene = CatchScene();	// shouldn't be exposed since it allows "active" plugs
	if(TheScene)	TheScene->GetCallbacksManager()->AddCallback(SNCB_POSTRENDER, TestCallback);
}

void ModuleDetach()
{
	RMScene* TheScene = CatchScene();
	if(TheScene)	TheScene->GetCallbacksManager()->RemoveCallback(SNCB_POSTRENDER, TestCallback);

	UnregisterPlugin(null, GetPlugin());
}

#endif
