///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the basic console commands.
 *	\file		IceBasicCmds.h
 *	\author		Pierre Terdiman
 *	\date		February, 22, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBASICCMDS_H
#define ICEBASICCMDS_H

#ifdef SUPPORT_CONSOLE

	void	BasicCmdDir			(Console* con, char* text, long user_data);
	void	BasicCmdCd			(Console* con, char* text, long user_data);
	void	BasicCmdexit		(Console* con, char* text, long user_data);
	void	BasicCmddestroy		(Console* con, char* text, long user_data);
	void	BasicCmdecho		(Console* con, char* text, long user_data);
	void	BasicCmdcls			(Console* con, char* text, long user_data);
	void	BasicCmdExecFile	(Console* con, char* text, long user_data);
	void	BasicCmdcmdlist		(Console* con, char* text, long user_data);
	void	BasicCmdcmdhist		(Console* con, char* text, long user_data);
	void	BasicCmdSetPrompt	(Console* con, char* text, long user_data);
	void	BasicCmdRotozoom	(Console* con, char* text, long user_data);

#endif

#endif // ICEBASICCMDS_H
