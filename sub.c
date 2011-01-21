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
* zdl.c : Window Procedures for minor dialogs
**********************************************************************/
#include "zdl.h"

LRESULT CALLBACK AboutProc(HWND Dlg,UINT Msg,WPARAM wp,LPARAM lp){if(Msg==WM_COMMAND&&LOWORD(wp)==BTN_OK){EndDialog(Dlg,0);}return 0;}

LRESULT CALLBACK AssocProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp){
	long sz=32;
	char tmp[64]={0};
	switch(msg){
		case WM_INITDIALOG:
			if(RegQueryValue(HKEY_CLASSES_ROOT,"ZDL.SaveFile",tmp,&sz)==ERROR_SUCCESS&&tmp[0]){SendMessage(GetDlgItem(dlg,CHK_ZDL),BM_SETCHECK,1,0);cfg.assoc[0]=1;}sz=MAX_PATH;
			if(RegQueryValue(HKEY_CLASSES_ROOT,"ZDL.WADFile",tmp,&sz)==ERROR_SUCCESS&&tmp[0]){SendMessage(GetDlgItem(dlg,CHK_WAD),BM_SETCHECK,1,0);cfg.assoc[1]=1;}sz=MAX_PATH;
			if(RegQueryValue(HKEY_CLASSES_ROOT,"ZDL.PatchFile",tmp,&sz)==ERROR_SUCCESS&&tmp[0]){SendMessage(GetDlgItem(dlg,CHK_DEH),BM_SETCHECK,1,0);cfg.assoc[2]=1;}
		break;
		case WM_COMMAND:switch(HIWORD(wp)){
			case BN_CLICKED:if(LOWORD(wp)==BTN_OK){
				// Save Files
				if(SendMessage(GetDlgItem(dlg,CHK_ZDL),BM_GETCHECK,0,0)){if(!cfg.assoc[0]){ // Set the value
					RegisterFileType(".zdl","ZDL.SaveFile","ZDL Saved Configuration File",_pgmptr,"\"%1\"",1);
				}}else{if(cfg.assoc[0]){ // Delete keys
					RegDeleteKey(HKEY_CLASSES_ROOT,".zdl");
					SHDeleteKey(HKEY_CLASSES_ROOT,"ZDL.SaveFile");
				}}
				if(SendMessage(GetDlgItem(dlg,CHK_WAD),BM_GETCHECK,0,0)){if(!cfg.assoc[1]){ // Set the value
					RegisterFileType(".wad","ZDL.WADFile","Doom Engine Data File",_pgmptr,"\"%1\"",2);
				}}else{if(cfg.assoc[1]){ // Delete keys
					RegDeleteKey(HKEY_CLASSES_ROOT,".wad");
					SHDeleteKey(HKEY_CLASSES_ROOT,"ZDL.WADFile");
				}}
				if(SendMessage(GetDlgItem(dlg,CHK_DEH),BM_GETCHECK,0,0)){if(!cfg.assoc[2]){ // Set the value
					RegisterFileType(".deh","ZDL.PatchFile","DeHackEd Patch",_pgmptr,"\"%1\"",3);
					RegisterFileType(".bex","ZDL.PatchFile","DeHackEd Patch",_pgmptr,"\"%1\"",3);
				}}else{if(cfg.assoc[2]){ // Delete keys
					RegDeleteKey(HKEY_CLASSES_ROOT,".deh");
					RegDeleteKey(HKEY_CLASSES_ROOT,".bex");
					SHDeleteKey(HKEY_CLASSES_ROOT,"ZDL.PatchFile");
				}}
				EndDialog(dlg,0);
			}break;
		}break;
	}return 0;
}

LRESULT CALLBACK FileProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp){
	FILE *fptr=0;
	ITEM **item=0;
	OPENFILENAME ofn={sizeof(OPENFILENAME),dlg,GetModuleHandle(NULL),0,0,0,0,0,MAX_PATH,0,0,0,0,OFN_FILEMUSTEXIST|OFN_EXPLORER,0,0,0,0,0,0};
	switch(msg){
		case WM_INITDIALOG:
			item=(arg1==LST_PORT)?(port):(iwad);
			if(arg2!=-1){ // Only set the text in the editboxes if it's not for a new entry
				SendMessage(dlg,WM_SETTEXT,0,(arg1==LST_PORT)?((LPARAM)"Edit Port"):((LPARAM)"Edit IWAD"));
				SendMessage(GetDlgItem(dlg,EDT_NAME),WM_SETTEXT,0,(LPARAM)item[arg2]->name);
				SendMessage(GetDlgItem(dlg,EDT_PATH),WM_SETTEXT,0,(LPARAM)item[arg2]->path);
			}else{SendMessage(dlg,WM_SETTEXT,0,(arg1==LST_PORT)?((LPARAM)"New Port"):((LPARAM)"New IWAD"));}
			SendMessage(GetDlgItem(dlg,EDT_NAME),EM_LIMITTEXT,MAX_NAME,0);
			SendMessage(GetDlgItem(dlg,EDT_PATH),EM_LIMITTEXT,MAX_PATH,0);
		break;
		case WM_COMMAND:switch(HIWORD(wp)){
			case BN_CLICKED:switch(LOWORD(wp)){
				case BTN_BROWSE: // Set up the path
					// Set up the file dialog
					memset((ofn.lpstrFile=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
					ofn.lpstrFilter=(arg1==LST_PORT)?("Executable Files (*.exe)\0*.exe"):("IWAD Files (*.wad)\0*.wad");
					ofn.lpstrDefExt=(arg1==LST_PORT)?("exe"):("wad");
					ofn.lpstrTitle=(arg1==LST_PORT)?("Browse for Port Executable"):("Browse for IWAD");
					// Get the file and set the editbox
					if(GetOpenFileName(&ofn)){SendMessage(GetDlgItem(dlg,EDT_PATH),WM_SETTEXT,0,(LPARAM)ofn.lpstrFile);}
					free(ofn.lpstrFile);
				break;
				case BTN_OK: // Save the changes
					item=(arg1==LST_PORT)?(port):(iwad);
					if(!SendMessage(GetDlgItem(dlg,EDT_NAME),WM_GETTEXTLENGTH,0,0)){MessageBox(dlg,"You must type a name.","Error",MB_OK|MB_ICONINFORMATION);break;}
					if(!SendMessage(GetDlgItem(dlg,EDT_PATH),WM_GETTEXTLENGTH,0,0)){MessageBox(dlg,"You must type a path.","Error",MB_OK|MB_ICONINFORMATION);break;}
					if(arg2==-1){ // If you are making a new item
						item[arg3]=malloc(sizeof(ITEM));
					}else{arg3=arg2;}
					SendMessage(GetDlgItem(dlg,EDT_NAME),WM_GETTEXT,MAX_NAME,(LPARAM)item[arg3]->name);
					SendMessage(GetDlgItem(dlg,EDT_PATH),WM_GETTEXT,MAX_PATH,(LPARAM)item[arg3]->path);
					if((fptr=fopen(item[arg3]->path,"r"))){fclose(fptr);
						item[arg3]->avail=1;
					}else{item[arg3]->avail=0;}
					arg1=arg2=arg3=0; // Reset the temp vars
				case BTN_EXIT:EndDialog(dlg,0);break;
			}break;
		}break;
	}return 0;
}
