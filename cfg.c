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
* cfg.c : Config file loading and saving stuff
**********************************************************************/
#include "zdl.h"

  ///////////////////////////////////////////////////////////////////
 // ReadINI - Reads an INI value to a buffer from a given file
//  Written by BioHazard on November 18th 2005 -- Version 1.3
//// USAGE //////////////////////////////////////////////////////////
// section : The INI section to start reading at (don't include []'s)
// entry   : The entry to read to [out] (don't include the '=')
// out     : Buffer to read the line at [entry] to
// limit   : Size of [out] in chars (overflowing output is truncated)
// ini     : Path to the file to be opened
//// RETURN /////////////////////////////////////////////////////////
// >=0 : Number of chars copied to [out]
//  -1 : [ini] could not be opened for reading
//  -2 : [section] could not be found
//  -3 : [entry] could not be found under [section]
//// REMARKS ////////////////////////////////////////////////////////
// [entry] must not start with '['
// <stdio.h> is required (and perhaps <string.h>)
int ReadINI(char *section,char *entry,char *out,int limit,char *ini){
	int i=0;
	FILE *fptr=0;
	char *tmp=0,*sct=0,*key=0;
	if(!(fptr=fopen(ini,"r"))){return -1;}
	// set up the compare strings
	i=(strlen(section)+3)*sizeof(char);
	memset((sct=malloc(i)),0,i);
	strcpy(sct,"[");strcat(sct,section);strcat(sct,"]"); // make [sct] = "[*section*]"
	i=(strlen(entry)+2)*sizeof(char);
	memset((key=malloc(i)),0,i);
	strcpy(key,entry);strcat(key,"="); // make [key] = "*entry*="
	i=(strlen((strlen(sct)>strlen(key))?(sct):(key))+1)*sizeof(char);
	memset((tmp=malloc(i)),0,i);
	// Get to the proper section
	do{fgets(tmp,strlen(sct)+1,fptr);
		if(feof(fptr)){i=-2;goto exit;} // Die if [section] can't be found
	}while(strcmp(tmp,sct));
	// Find the right entry
	do{fgets(tmp,strlen(key)+1,fptr);
		if(feof(fptr)||tmp[0]=='['){i=-3;goto exit;} // Die if [entry] can't be found in [section]
	}while(strcmp(tmp,key));
	// Copy the string to the buffer
	fgets(out,limit,fptr); // Read the option to the buffer
	if(out[strlen(out)-1]=='\n'){out[strlen(out)-1]='\0';} // chop off the \n if there is one
	i=strlen(out);
exit: // Free crap and exit
	fclose(fptr);free(tmp);free(sct);free(key);
	return i;
}

  ///////////////////////////////////////////////////////////////////
 // RegisterFileType - Registers a filetype with Windows
//  Written by BioHazard on November 30th 2005 -- Version 1.0
//// USAGE //////////////////////////////////////////////////////////
// ext      : The extension to register
// type     : The data type to associate with
// nicetype : Description of the data type
// exe      : Executable to run the files with
// command  : command line to stick after [exe]
// icon     : 0-based index of icon in [exe] to be uesd for files
//// REMARKS ////////////////////////////////////////////////////////
// Testen to work in WinXP-Pro and Win2000
void RegisterFileType(char *ext,char *type,char *nicetype,char *exe,char* command,int icon){
	int i=0;
	char *tmp=0,*tmp2=0;
	i=strlen(command)+strlen(exe)+MAX_PATH;
	memset((tmp=malloc(i*sizeof(char))),0,i*sizeof(char));
	memset((tmp2=malloc(i*sizeof(char))),0,i*sizeof(char));
	// Delete the old extensions
	_snprintf(tmp,i,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s",ext);
	SHDeleteKey(HKEY_CURRENT_USER,tmp);
	SHDeleteKey(HKEY_CLASSES_ROOT,ext);
	SHDeleteKey(HKEY_CLASSES_ROOT,type);
	// Add new keys
	RegSetValue(HKEY_CLASSES_ROOT,ext,REG_SZ,type,strlen(type));
	RegSetValue(HKEY_CLASSES_ROOT,type,REG_SZ,nicetype,strlen(nicetype));
	_snprintf(tmp,i,"%s\\DefaultIcon",type);
	_snprintf(tmp2,i,"%s,%d",_pgmptr,icon);
	RegSetValue(HKEY_CLASSES_ROOT,tmp,REG_SZ,tmp2,strlen(tmp2));
	_snprintf(tmp,i,"%s\\shell\\open\\command",type);
	_snprintf(tmp2,i,"\"%s\" %s",exe,command);
	RegSetValue(HKEY_CLASSES_ROOT,tmp,REG_SZ,tmp2,strlen(tmp2));
	// Free crap
	free(tmp);free(tmp2);
}

 /////////////////////////////////////////////////
// Cfg_LoadConfig : Loads data from the INI file
void Cfg_LoadConfig(){
	int i=0;
	FILE *fptr=0;
	char tmp[8]={0};
	ReadINI("zdl.general","alwaysadd",cfg.always,sizeof(cfg.always),cfg.ini);
	ReadINI("zdl.general","zdllaunch",tmp,sizeof(tmp),cfg.ini);
	if(tmp[0]=='1'){cfg.launch=1;}else{cfg.launch=0;}
	// Load ports
	for(i=0;i<MAX_ITEM;i++){
		port[i] = malloc(sizeof(ITEM));
		_snprintf(tmp,sizeof(tmp),"p%dn",i);
		if(ReadINI("zdl.ports",tmp,port[i]->name,MAX_NAME,cfg.ini)<0){free(port[i]);port[i]=0;break;}
		_snprintf(tmp,sizeof(tmp),"p%df",i);
		if(ReadINI("zdl.ports",tmp,port[i]->path,MAX_PATH,cfg.ini)<0){free(port[i]);port[i]=0;break;}
		if((fptr=fopen(port[i]->path,"r"))){
			fclose(fptr);
			port[i]->avail=1;
		}
	}
	// Load IWADs
	for(i=0;i<MAX_ITEM;i++){
		iwad[i] = malloc(sizeof(ITEM));
		_snprintf(tmp,sizeof(tmp),"i%dn",i);
		if(ReadINI("zdl.iwads",tmp,iwad[i]->name,MAX_NAME,cfg.ini)<0){free(iwad[i]);iwad[i]=0;break;}
		_snprintf(tmp,sizeof(tmp),"i%df",i);
		if(ReadINI("zdl.iwads",tmp,iwad[i]->path,MAX_PATH,cfg.ini)<0){free(iwad[i]);iwad[i]=0;break;}
		if((fptr=fopen(iwad[i]->path,"r"))){
			fclose(fptr);
			iwad[i]->avail=1;
		}
	}
}

 /////////////////////////////////////////
// Cfg_GetSel : Gets the Port/IWAD index
int Cfg_GetSel(int sel,ITEM **item){
	int i=0,q=0;
	for(i=0;q<=sel;i++){if(!item[i]){return -1;}
		if(item[i]->avail){q++;}
	}
	return i-1;
}

 /////////////////////////////////////////////////
// Cfg_GetSelStr : Gets a selection index from a name
int Cfg_GetSelStr(char *name,ITEM **item){
	int i=0,q=0,r=0;
	for(i=0;;i++){ // Get to the real index
		if(!item[i]||i>=MAX_ITEM){return -2;}
		if(!stricmp(name,item[i]->name)){break;}
	}
	if(!item[i]->avail){return -1;} // Make sure the found entry is available
	for(q=0;q<i;q++){if(item[q]->avail){r++;}} // Figure out what the list index is
	return r;
}

 /////////////////////////////////////////////////
// Cfg_ReadSave : Reads a save file and applies it's settings
int Cfg_ReadSave(HWND dlg,char *file){
	int i=0,q=0;
	char *tmp=0,*tmp2=0;
	int boxid[2][6]={{LST_WARP,EDT_EXTRA,EDT_HOST,EDT_FRAGS,EDT_DMF,EDT_DMF2},{LST_SKILL,LST_GAME,LST_PLAYERS}};
	char *box[2][6]={{"warp","extra","host","fraglimit","dmflags","dmflags2"},{"skill","gametype","players"}};
	if(Cfg_GetSel(0,port)==-1||Cfg_GetSel(0,iwad)==-1){MessageBox(dlg,STR_NOITEMS,"Error!",MB_OK|MB_ICONEXCLAMATION);return 0;}
	memset((tmp=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
	memset((tmp2=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
// Try to find [zdl.save] to see if this is a save file
	if(ReadINI("zdl.save","keyb",tmp,MAX_PATH,file)==-2){
		MessageBox(dlg,"This save file is an invalid format.","Error!",MB_OK|MB_ICONEXCLAMATION);
		i=0;goto exit;
	}
// Start reading the save data
	// Port
	if(ReadINI("zdl.save","port",tmp,MAX_PATH,file)>0){
		if(!port[0]||(i=Cfg_GetSelStr(tmp,port))<0){
			_snprintf(tmp2,MAX_PATH,STR_NEEDPRT,tmp,port[Cfg_GetSel(0,port)]->name);
			if(MessageBox(dlg,tmp2,"Port not found!",MB_YESNO|MB_ICONQUESTION)==IDNO){i=0;goto exit;}
			i=0;
		}
	}
	// Iwad
	if(ReadINI("zdl.save","iwad",tmp,MAX_PATH,file)>0){
		if(!iwad[0]||(q=Cfg_GetSelStr(tmp,iwad))<0){
			_snprintf(tmp2,MAX_PATH,STR_NEEDIWD,tmp,iwad[Cfg_GetSel(0,iwad)]->name);
			if(MessageBox(dlg,tmp2,"Load Failed!",MB_YESNO|MB_ICONQUESTION)==IDNO){i=0;goto exit;}
			q=0;
		}
	}
	Dlg_ClearAll(dlg);
	SendMessage(GetDlgItem(dlg,LST_PORT),CB_SETCURSEL,i,0);
	SendMessage(GetDlgItem(dlg,LST_IWAD),LB_SETCURSEL,q,0);
	Dlg_PopulateWarp(dlg,iwad[Cfg_GetSel(q,iwad)]->path);
	// EditBoxes (Warp, Extra, Host, Frags, DMF, DMF2)
	for(i=0;i<6;i++){if(ReadINI("zdl.save",box[0][i],tmp,MAX_PATH,file)>0){
		SendMessage(GetDlgItem(dlg,boxid[0][i]),WM_SETTEXT,0,(LPARAM)tmp);
	}}
	// ComboBoxes (Skill, GameType, Players)
	for(i=0;i<3;i++){if(ReadINI("zdl.save",box[1][i],tmp,MAX_PATH,file)>0){
		SendMessage(GetDlgItem(dlg,boxid[1][i]),CB_SETCURSEL,(WPARAM)atoi(tmp),0);
	}}
	// Dialog Mode
	if(ReadINI("zdl.save","dlgmode",tmp,MAX_PATH,file)>0&&!stricmp(tmp,"open")&&cfg.dlgmode){
		SendMessage(dlg,WM_COMMAND,MAKELONG(BTN_PANEL,BN_CLICKED),0);
	}
	// PWAD list
	for(i=0;i<MAX_PWAD;i++){
		_snprintf(tmp2,MAX_PATH,"file%d",q);
		if(ReadINI("zdl.save",tmp2,tmp,MAX_PATH,file)>0){
			if(!Dlg_AddPWAD(dlg,tmp)){i--;}
		}else{break;}
		if(i>=MAX_PWAD){MessageBox(dlg,STR_MAXPWAD,"Error!",MB_OK|MB_ICONEXCLAMATION);break;}
	q++;}i=1;
exit: // Free crap and exit
	free(tmp);free(tmp2);
	return i;
}

 /////////////////////////////////////////////////
// Cfg_WriteSave : Writes dialog data to the file given
void Cfg_WriteSave(HWND dlg,FILE *fptr){
	int i=0;
	char *tmp=0;
	// Used for the box grabber
	int boxid[]={EDT_EXTRA,EDT_HOST,EDT_FRAGS,EDT_DMF,EDT_DMF2};
	char *box[]={"extra=%s\n","host=%s\n","fraglimit=%s\n","dmflags=%s\n","dmflags2=%s\n"};
	memset((tmp=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
	fprintf(fptr,"[zdl.save]\n");
// Standard stuff
	if(SendMessage(GetDlgItem(dlg,LST_PORT),CB_GETCURSEL,0,0)!=LB_ERR){fprintf(fptr,"port=%s\n",port[Cfg_GetSel(SendMessage(GetDlgItem(dlg,LST_PORT),CB_GETCURSEL,0,0),port)]->name);}
	if(SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCURSEL,0,0)!=LB_ERR){fprintf(fptr,"iwad=%s\n",iwad[Cfg_GetSel(SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCURSEL,0,0),iwad)]->name);}
	if(!cfg.dlgmode){fputs("dlgmode=open\n",fptr);}
	SendMessage(GetDlgItem(dlg,LST_WARP),WM_GETTEXT,MAX_PATH,(LPARAM)tmp);
	if(tmp[0]&&stricmp(tmp,"NONE")){fprintf(fptr,"warp=%s\nskill=%d\n",tmp,SendMessage(GetDlgItem(dlg,LST_SKILL),CB_GETCURSEL,0,0));}
	if((i=SendMessage(GetDlgItem(dlg,LST_GAME),CB_GETCURSEL,0,0))){fprintf(fptr,"gametype=%d\n",i);}
	if((i=SendMessage(GetDlgItem(dlg,LST_PLAYERS),CB_GETCURSEL,0,0))){fprintf(fptr,"players=%d\n",i);}
	for(i=0;i<5;i++){ // Grab text from the boxes
		if(SendMessage(GetDlgItem(dlg,boxid[i]),WM_GETTEXTLENGTH,0,0)){
			SendMessage(GetDlgItem(dlg,boxid[i]),WM_GETTEXT,MAX_PATH,(LPARAM)tmp);
			fprintf(fptr,box[i],tmp);
		}
	}
// PWADs
	for(i=0;pwad[i]&&i<MAX_PWAD;i++){fprintf(fptr,"file%d=%s\n",i,pwad[i]);}
	free(tmp);
}
