/**********************************************************************
* ZDL!
* Copyright (C) 2005 BioHazard / Vectec Software
***********************************************************************
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 or any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
***********************************************************************
* dlg.c : Most of the important functions are here
**********************************************************************/
#include "zdl.h"

 /////////////////////////////////////////////////
// Dlg_Launch : The most important function in ZDL
void Dlg_Launch(HWND dlg,char prompt){
	int i=0,q=MAX_PATH*3,pw=0,pt=0;
	char *cmd=0,*tmp=0;
	char *portExe;
	// Make sure there is a port selected
	if(!SendMessage(GetDlgItem(dlg,LST_PORT),CB_GETCOUNT,0,0)||!SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCOUNT,0,0)){MessageBox(dlg,STR_NOITEMS,"Error!",MB_OK|MB_ICONEXCLAMATION);return;}
	// Determine how long the cmd string has to be
	for(i=0;pwad[i]&&i<MAX_PWAD;i++){q+=strlen(pwad[i])+4;} // Count the length of the PWADs
	// Alloc the string
	memset((tmp=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
	memset((cmd=malloc(q*sizeof(char))),0,q*sizeof(char));
	// Print the basic stuff
	portExe = port[Cfg_GetSel(SendMessage(GetDlgItem(dlg,LST_PORT),CB_GETCURSEL,0,0),port)]->path;
	_snprintf(cmd,q,"\"%s\" -iwad \"%s\"%s%s",portExe,iwad[Cfg_GetSel(SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCURSEL,0,0),iwad)]->path,(strlen(cfg.always))?(" "):(""),cfg.always);
	// Warp and Skill
	SendMessage(GetDlgItem(dlg,LST_WARP),WM_GETTEXT,MAX_PATH,(LPARAM)tmp);
	if(stricmp(tmp,"")){
		strncat(cmd," +map ",q-strlen(cmd));
		strncat(cmd,tmp,q-strlen(cmd));
		_snprintf(&cmd[strlen(cmd)],q-strlen(cmd)," -skill %d",SendMessage(GetDlgItem(dlg,LST_SKILL),CB_GETCURSEL,0,0)+1);
	}
	// PWADs and DEHs
	for(i=0;pwad[i]&&i<MAX_PWAD;i++){ // Count up each type of file
		if(stricmp(strrchr(pwad[i],'.'),".deh")&&stricmp(strrchr(pwad[i],'.'),".bex")){pw++;}else{pt++;}
	}
	if(pw){ // Append PWADs
		strncat(cmd," -file",q-strlen(cmd));
		for(i=0;pwad[i]&&i<MAX_PWAD;i++){
			if(stricmp(strrchr(pwad[i],'.'),".deh")&&stricmp(strrchr(pwad[i],'.'),".bex")){
				_snprintf(&cmd[strlen(cmd)],q-strlen(cmd)," \"%s\"",pwad[i]);
			}
		}
	}
	if(pt){ // Append DEHs
		strncat(cmd," -deh",q-strlen(cmd));
		for(i=0;pwad[i]&&i<MAX_PWAD;i++){
			if(!stricmp(strrchr(pwad[i],'.'),".deh")||!stricmp(strrchr(pwad[i],'.'),".bex")){
				_snprintf(&cmd[strlen(cmd)],q-strlen(cmd)," \"%s\"",pwad[i]);
			}
		}
	}
	// Extra Args
	if(SendMessage(GetDlgItem(dlg,EDT_EXTRA),WM_GETTEXTLENGTH,0,0)){
		strncat(cmd," ",q-strlen(cmd));
		SendMessage(GetDlgItem(dlg,EDT_EXTRA),WM_GETTEXT,q-strlen(cmd),(LPARAM)&cmd[strlen(cmd)]);
	}
	// Network stuff
	if(SendMessage(GetDlgItem(dlg,LST_GAME),CB_GETCURSEL,0,0)){
		if((i=SendMessage(GetDlgItem(dlg,LST_PLAYERS),CB_GETCURSEL,0,0))){ // Hosting
			_snprintf(&cmd[strlen(cmd)],q-strlen(cmd)," -host %d",i);
			// Deathmatch Flag
			if(SendMessage(GetDlgItem(dlg,LST_GAME),CB_GETCURSEL,0,0)==2){
				strncat(cmd," -deathmatch",q-strlen(cmd));
				// Fragimit
				if(SendMessage(GetDlgItem(dlg,EDT_FRAGS),WM_GETTEXTLENGTH,0,0)){
					strncat(cmd," +fraglimit ",q-strlen(cmd));
					SendMessage(GetDlgItem(dlg,EDT_FRAGS),WM_GETTEXT,q-strlen(cmd),(LPARAM)&cmd[strlen(cmd)]);
				}
			}
			// DMF
			if(SendMessage(GetDlgItem(dlg,EDT_DMF),WM_GETTEXTLENGTH,0,0)){
				strncat(cmd," +dmflags ",q-strlen(cmd));
				SendMessage(GetDlgItem(dlg,EDT_DMF),WM_GETTEXT,q-strlen(cmd),(LPARAM)&cmd[strlen(cmd)]);
			}
			// DMF2
			if(SendMessage(GetDlgItem(dlg,EDT_DMF2),WM_GETTEXTLENGTH,0,0)){
				strncat(cmd," +dmflags2 ",q-strlen(cmd));
				SendMessage(GetDlgItem(dlg,EDT_DMF2),WM_GETTEXT,q-strlen(cmd),(LPARAM)&cmd[strlen(cmd)]);
			}
		}else{ // Joining
			if(!SendMessage(GetDlgItem(dlg,EDT_HOST),WM_GETTEXTLENGTH,0,0)){MessageBox(dlg,"You need to type a host to join","Error!",MB_OK|MB_ICONHAND);free(cmd);return;}
			strcat(cmd," -join ");
			SendMessage(GetDlgItem(dlg,EDT_HOST),WM_GETTEXT,q-strlen(cmd),(LPARAM)&cmd[strlen(cmd)]);
		}
	}
	if(prompt&&MessageBox(dlg,cmd,"Run this command-line?",MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION)==IDNO){goto exit;}
	// Change to the port directory
	if(strrchr(portExe,'\\')){
		strncpy(tmp,portExe,MAX_PATH);
		strrchr(tmp,'\\')[0]='\0';
		chdir(tmp);
	}
	// GoooOOooooOOOOOoooOOO!
	WinExec(cmd,SW_NORMAL);
	// Quit on launch?
	if(cfg.autoclose){
		Dlg_Quit(dlg,1);
	}
exit:
	free(cmd);free(tmp);
}

/////////////////////////////////////////////////
// Dlg_AddPWAD : Adds an item to the PWAD list
int Dlg_AddPWAD(HWND dlg,char *file){
	int i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCOUNT,0,0);
	if(i>=MAX_PWAD||!strrchr(file,'.')||!stricmp(strrchr(file,'.'),".zdl")||!stricmp(strrchr(file,'.'),".ini")){return 0;}
	memset((pwad[i]=malloc((MAX_PATH+1)*sizeof(char))),0,MAX_PATH*sizeof(char));
	strncpy(pwad[i],file,MAX_PATH);
	SendMessage(GetDlgItem(dlg,LST_PWAD),LB_ADDSTRING,0,(strrchr(file,'\\'))?((LPARAM)strrchr(file,'\\')+1):((LPARAM)file));
	return 1;
}
 /////////////////////////////////////////////////
// Dlg_ClearPWAD : Clears all the data from the PWAD list
void Dlg_ClearPWAD(HWND dlg){
	int i=0;
	SendMessage(GetDlgItem(dlg,LST_PWAD),LB_RESETCONTENT,0,0);
	for(i=0;pwad[i]&&i<MAX_PWAD;i++){free(pwad[i]);pwad[i]=0;}
}

 /////////////////////////////////////////////////
// Dlg_ClearAll : Clears all the data from the dialog
void Dlg_ClearAll(HWND dlg){
	int i=0;
	int box[2][5]={{LST_PORT,LST_GAME,LST_PLAYERS},{EDT_EXTRA,EDT_HOST,EDT_FRAGS,EDT_DMF,EDT_DMF2}};
	Dlg_ClearPWAD(dlg);
	SendMessage(GetDlgItem(dlg,LST_IWAD),LB_SETCURSEL,0,0);
	SendMessage(GetDlgItem(dlg,LST_WARP),WM_SETTEXT,0,(LPARAM)"");
	SendMessage(GetDlgItem(dlg,LST_SKILL),CB_SETCURSEL,2,0);
	for(i=0;i<3;i++){SendMessage(GetDlgItem(dlg,box[0][i]),CB_SETCURSEL,0,0);}
	for(i=0;i<5;i++){SendMessage(GetDlgItem(dlg,box[1][i]),WM_SETTEXT,0,(LPARAM)"");}
}

 /////////////////////////////////////////////////
// Dlg_Quit : Saves data and exits ZDL
void Dlg_Quit(HWND dlg,char save){
	int i=0;
	FILE *fptr=0;
	// Save the config
	if(save){
		if(!cfg.always[0]&&!cfg.launch&&!port[0]&&!iwad[0]){ // Do we even need to save at all?
			Dlg_ClearPWAD(dlg);
			EndDialog(dlg,0);return;
		}
		if(!(fptr=fopen(cfg.ini,"w"))){MessageBox(HWND_DESKTOP,"The INI file could not be opened for writing!\nYour settings have not been saved!","Error!",MB_OK|MB_ICONHAND);return;}
		if(cfg.always[0]||cfg.launch||cfg.autoclose){
			fputs("[zdl.general]\n",fptr);
			if(cfg.always[0]){fprintf(fptr,"alwaysadd=%s\n",cfg.always);}
			if(cfg.launch){fputs("zdllaunch=1\n",fptr);}
			if(cfg.autoclose){fputs("autoclose=1\n",fptr);}
		}
		if(port[0]){ // Write ports
			fprintf(fptr,"[zdl.ports]\n");
			for(i=0;port[i]&&i<MAX_ITEM;i++){fprintf(fptr,"p%dn=%s\np%df=%s\n",i,port[i]->name,i,port[i]->path);}
		}
		if(iwad[0]){ // Write IWADs
			fprintf(fptr,"[zdl.iwads]\n");
			for(i=0;iwad[i]&&i<MAX_ITEM;i++){fprintf(fptr,"i%dn=%s\ni%df=%s\n",i,iwad[i]->name,i,iwad[i]->path);}
		}
		Cfg_WriteSave(dlg,fptr);
		fclose(fptr);
	}
	// Delete port and IWAD data
	for(i=0;port[i]&&i<MAX_ITEM;i++){free(port[i]);}
	for(i=0;iwad[i]&&i<MAX_ITEM;i++){free(iwad[i]);}
	Dlg_ClearPWAD(dlg);
	EndDialog(dlg,0);
}

 /////////////////////////////////////////////////
// PopulateWarp : Reads through the wad and finds valid maps
void Dlg_PopulateWarp(HWND dlg,char *file){
	unsigned int i=0;
	char temp[9]={0};
	WADHEAD header;
	LUMPHEAD lump[2];
	FILE *fptr=0;
	SendMessage(GetDlgItem(dlg,LST_WARP),CB_RESETCONTENT,0,0);
	SendMessage(GetDlgItem(dlg,LST_WARP),CB_ADDSTRING,0,(LPARAM)"");
	if((fptr=fopen(file,"rb"))){
		fread(&header,1,sizeof(WADHEAD),fptr);
		fseek(fptr,header.dir,SEEK_SET);
		for(i=0;i<header.lumps;i++){
			fread(&lump,2,sizeof(LUMPHEAD),fptr);
			if(!stricmp(lump[1].name,"THINGS")){
				strncpy(temp,lump[0].name,8);
				SendMessage(GetDlgItem(dlg,LST_WARP),CB_ADDSTRING,0,(LPARAM)temp);
			}else{fseek(fptr,sizeof(LUMPHEAD)-(sizeof(LUMPHEAD)*2),SEEK_CUR);}
		}
		SendMessage(GetDlgItem(dlg,LST_WARP),CB_SETCURSEL,0,0);
		fclose(fptr);
	}
}
