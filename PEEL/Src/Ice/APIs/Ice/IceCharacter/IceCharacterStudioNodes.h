///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Standard wordlist used in Character Studio 2.0
 *	\file		IceCharacterStudio.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECHARACTERSTUDIONODES_H
#define ICECHARACTERSTUDIONODES_H

	// Note: this file belongs to ICE's character-related dll. A carbon-copy may be found in Flexporter.

	#define BIPED_MAX_NB_NODES	105		//!< Total possible number of bones

	typedef udword	CSID;				//!< Character Studio ID

	// Useful CSIDs
	enum CSID_Index
	{
		CSID_ROOT		=	0,			//!< CSID for the root node
		CSID_PELVIS		=	2,			//!< CSID for the pelvis
		CSID_SPINE		=	3,			//!< CSID for the spine
		//
		CSID_NECK		=	8,			//!< CSID for the neck
		CSID_HEAD		=	13,			//!< CSID for the head
		//
		CSID_LFOOT		=	65,			//!< CSID for the left foot
		CSID_RFOOT		=	84,			//!< CSID for the right foot
		CSID_LCALF		=	63,			//!< CSID for the left calf
		CSID_RCALF		=	82,			//!< CSID for the right calf
		CSID_LTHIGH		=	62,			//!< CSID for the left thigh
		CSID_RTHIGH		=	81,			//!< CSID for the right thigh
		//
		CSID_LHAND		=	27,			//!< CSID for the left hand
		CSID_RHAND		=	46,			//!< CSID for the right hand
		CSID_LELBOW		=	26,			//!< CSID for the left elbow
		CSID_RELBOW		=	45,			//!< CSID for the right elbow
		CSID_LSHOULDER	=	25,			//!< CSID for the left shoulder
		CSID_RSHOULDER	=	44,			//!< CSID for the right shoulder
		CSID_LCLAVICLE	=	24,			//!< CSID for the left clavicle
		CSID_RCLAVICLE	=	43,			//!< CSID for the right clavicle
		//
//		CSID_NONE		=	0xff		//!< Invalid CSID
		CSID_INVALID	=	0xff		//!< Invalid CSID
	};

	// What I call a "CSID" (Character Studio ID) is a constant number between 0 and 104, as described in the list below.
	// Each skeleton (and each character) must use that wordlist in order to work with ICE's Granny-like character animation system.

	static char CharacterStudioNames[] = {
	"Bip01\n"							// 0		// Root node
	"Bip01 Footsteps\n"					// 1		// Pas
	"Bip01 Pelvis\n"					// 2		// Bassin
	"Bip01 Spine\n"						// 3		// Colonne vertébrale
	"Bip01 Spine1\n"					// 4
	"Bip01 Spine2\n"					// 5
	"Bip01 Spine3\n"					// 6
	"Bip01 Spine4\n"					// 7
	"Bip01 Neck\n"						// 8		// Cou
	"Bip01 Neck1\n"						// 9
	"Bip01 Neck2\n"						// 10
	"Bip01 Neck3\n"						// 11
	"Bip01 Neck4\n"						// 12
	"Bip01 Head\n"						// 13		// Tête
	"Bip01 Ponytail1\n"					// 14		// Queue de cheval
	"Bip01 Ponytail11\n"				// 15
	"Bip01 Ponytail12\n"				// 16
	"Bip01 Ponytail13\n"				// 17
	"Bip01 Ponytail14\n"				// 18
	"Bip01 Ponytail2\n"					// 19
	"Bip01 Ponytail21\n"				// 20
	"Bip01 Ponytail22\n"				// 21
	"Bip01 Ponytail23\n"				// 22
	"Bip01 Ponytail24\n"				// 23
	// Partie gauche
	"Bip01 L Clavicle\n"				// 24		// Clavicule
	"Bip01 L UpperArm\n"				// 25		// Bras supérieur
	"Bip01 L Forearm\n"					// 26		// Avant-bras
	"Bip01 L Hand\n"					// 27		// Main
	"Bip01 L Finger0\n"					// 28		// Doigts
	"Bip01 L Finger01\n"				// 29
	"Bip01 L Finger02\n"				// 30
	"Bip01 L Finger1\n"					// 31
	"Bip01 L Finger11\n"				// 32
	"Bip01 L Finger12\n"				// 33
	"Bip01 L Finger2\n"					// 34
	"Bip01 L Finger21\n"				// 35
	"Bip01 L Finger22\n"				// 36
	"Bip01 L Finger3\n"					// 37
	"Bip01 L Finger31\n"				// 38
	"Bip01 L Finger32\n"				// 39
	"Bip01 L Finger4\n"					// 40
	"Bip01 L Finger41\n"				// 41
	"Bip01 L Finger42\n"				// 42
	// Partie droite
	"Bip01 R Clavicle\n"				// 43
	"Bip01 R UpperArm\n"				// 44
	"Bip01 R Forearm\n"					// 45
	"Bip01 R Hand\n"					// 46
	"Bip01 R Finger0\n"					// 47
	"Bip01 R Finger01\n"				// 48
	"Bip01 R Finger02\n"				// 49
	"Bip01 R Finger1\n"					// 50
	"Bip01 R Finger11\n"				// 51
	"Bip01 R Finger12\n"				// 52
	"Bip01 R Finger2\n"					// 53
	"Bip01 R Finger21\n"				// 54
	"Bip01 R Finger22\n"				// 55
	"Bip01 R Finger3\n"					// 56
	"Bip01 R Finger31\n"				// 57
	"Bip01 R Finger32\n"				// 58
	"Bip01 R Finger4\n"					// 59
	"Bip01 R Finger41\n"				// 60
	"Bip01 R Finger42\n"				// 61
	// Partie gauche
	"Bip01 L Thigh\n"					// 62		// Cuisse
	"Bip01 L Calf\n"					// 63		// Mollet
	"Bip01 L HorseLink\n"				// 64
	"Bip01 L Foot\n"					// 65		// Pied
	"Bip01 L Toe0\n"					// 66		// Doigts de pied
	"Bip01 L Toe01\n"					// 67
	"Bip01 L Toe02\n"					// 68
	"Bip01 L Toe1\n"					// 69
	"Bip01 L Toe11\n"					// 70
	"Bip01 L Toe12\n"					// 71
	"Bip01 L Toe2\n"					// 72
	"Bip01 L Toe21\n"					// 73
	"Bip01 L Toe22\n"					// 74
	"Bip01 L Toe3\n"					// 75
	"Bip01 L Toe31\n"					// 76
	"Bip01 L Toe32\n"					// 77
	"Bip01 L Toe4\n"					// 78
	"Bip01 L Toe41\n"					// 79
	"Bip01 L Toe42\n"					// 80
	// Partie droite
	"Bip01 R Thigh\n"					// 81
	"Bip01 R Calf\n"					// 82
	"Bip01 R HorseLink\n"				// 83
	"Bip01 R Foot\n"					// 84
	"Bip01 R Toe0\n"					// 85
	"Bip01 R Toe01\n"					// 86
	"Bip01 R Toe02\n"					// 87
	"Bip01 R Toe1\n"					// 88
	"Bip01 R Toe11\n"					// 89
	"Bip01 R Toe12\n"					// 90
	"Bip01 R Toe2\n"					// 91
	"Bip01 R Toe21\n"					// 92
	"Bip01 R Toe22\n"					// 93
	"Bip01 R Toe3\n"					// 94
	"Bip01 R Toe31\n"					// 95
	"Bip01 R Toe32\n"					// 96
	"Bip01 R Toe4\n"					// 97
	"Bip01 R Toe41\n"					// 98
	"Bip01 R Toe42\n"					// 99

	"Bip01 Tail\n"						// 100		// Queue
	"Bip01 Tail1\n"						// 101
	"Bip01 Tail2\n"						// 102
	"Bip01 Tail3\n"						// 103
	"Bip01 Tail4\n"						// 104
	};

	// Maps a CSID to the parent CSID in the complete Character Studio hierarchy.
	static sdword CSIDToParentCSID[] =
	{
		-1, 0, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 13, 19, 20, 21, 22, 8, 24,
		25, 26, 27, 28, 29, 27, 31, 32, 27, 34, 35, 27, 37, 38, 27, 40, 41, 8, 43, 44, 45, 46, 47, 48,
		46, 50, 51, 46, 53, 54, 46, 56, 57, 46, 59, 60, 3, 62, 63, 64, 65, 66, 67, 65, 69, 70, 65, 72,
		73, 65, 75, 76, 65, 78, 79, 3, 81, 82, 83, 84, 85, 86, 84, 88, 89, 84, 91, 92, 84, 94, 95, 84,
		97, 98, 3, 100, 101, 102, 103
	};

#endif // ICECHARACTERSTUDIONODES_H

