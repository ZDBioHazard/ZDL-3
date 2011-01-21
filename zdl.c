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
* zdl.c : Entry point, System vars and Window Procedures
**********************************************************************/
#include "zdl.h"

// Global data
CFG cfg={0};
ITEM *iwad[MAX_ITEM+1]={0},*port[MAX_ITEM+1]={0};
char *pwad[MAX_PWAD+1]={0};
char *cmdline=0;
short arg1=0,arg2=0,arg3=0; // Needed for the config dialogs

LRESULT CALLBACK ConfigProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp){
	int i=0,q=0;
	ITEM *tmpi=0;
	ITEM **item=0;
	FILE *fptr=0;
	char *tmp=0;
	switch(msg){
		case WM_INITDIALOG:
		// Set icons
			SendMessage(GetDlgItem(dlg,BTN_UP),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_UP)));
			SendMessage(GetDlgItem(dlg,BTN_DWN),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_DOWN)));
			SendMessage(GetDlgItem(dlg,BTN_IUP),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_UP)));
			SendMessage(GetDlgItem(dlg,BTN_IDWN),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_DOWN)));
		// Set text limits
			SendMessage(GetDlgItem(dlg,EDT_EXTRA),EM_LIMITTEXT,sizeof(cfg.always),0);
		// Fill the dialog with info
			SendMessage(GetDlgItem(dlg,EDT_EXTRA),WM_SETTEXT,0,(LPARAM)cfg.always);
			SendMessage(GetDlgItem(dlg,CHK_LAUNCH),BM_SETCHECK,cfg.launch,0);
			SendMessage(GetDlgItem(dlg,CHK_AUTOCLOSE),BM_SETCHECK,cfg.autoclose,0);
			// Port and IWAD lists
			for(i=0;port[i]&&i<MAX_ITEM;i++){SendMessage(GetDlgItem(dlg,LST_PORT),LB_ADDSTRING,0,(LPARAM)port[i]->name);}
			for(i=0;iwad[i]&&i<MAX_ITEM;i++){SendMessage(GetDlgItem(dlg,LST_IWAD),LB_ADDSTRING,0,(LPARAM)iwad[i]->name);}
		break;
		case WM_COMMAND:switch(HIWORD(wp)){
			case LBN_DBLCLK:switch(LOWORD(wp)){
				case LST_PORT:case LST_IWAD:
					arg1=q=LOWORD(wp);
					arg2=i=(short)SendMessage(GetDlgItem(dlg,LOWORD(wp)),LB_GETCURSEL,0,0);
					item=(LOWORD(wp)==LST_PORT)?(port):(iwad);
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_FILE),dlg,FileProc);
					// Update the string in the listbox
					SendMessage(GetDlgItem(dlg,q),LB_DELETESTRING,i,0);
					SendMessage(GetDlgItem(dlg,q),LB_INSERTSTRING,i,(LPARAM)item[i]->name);
					SendMessage(GetDlgItem(dlg,q),LB_SETCURSEL,q,0);
					// Update the file's availabillity
					if((fptr=fopen(item[i]->path,"r"))){fclose(fptr);item[i]->avail=1;}else{item[i]->avail=0;}
					arg1=arg2=0; // Reset the temp vars
				break;
			}break;
			case BN_CLICKED:switch(LOWORD(wp)){
				case BTN_ZDL:DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_ASSOC),dlg,AssocProc);break;
				case BTN_ADD:case BTN_IADD:
					item=(LOWORD(wp)==BTN_ADD)?(port):(iwad);
					arg1=q=(LOWORD(wp)==BTN_ADD)?(LST_PORT):(LST_IWAD);
					arg2=-1; // -1 == BTN_ADD pressed
					arg3=i=(short)SendMessage(GetDlgItem(dlg,q),LB_GETCOUNT,0,0);
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_FILE),dlg,FileProc);
					// If a new entry was added, add it to the listbox
					if(item[i]){SendMessage(GetDlgItem(dlg,q),LB_ADDSTRING,0,(LPARAM)item[i]->name);}
					arg1=arg2=arg3=0; // Reset the temp vars
				break;
				case BTN_REM:case BTN_IREM: // Delete button
					item=(LOWORD(wp)==BTN_REM)?(port):(iwad);
					q=(LOWORD(wp)==BTN_REM)?(LST_PORT):(LST_IWAD);
					if((i=SendMessage(GetDlgItem(dlg,q),LB_GETCURSEL,0,0))!=LB_ERR){
						free(item[i]);
						SendMessage(GetDlgItem(dlg,q),LB_DELETESTRING,i,0);
						SendMessage(GetDlgItem(dlg,q),LB_SETCURSEL,i,0);
						for(i;item[i]&&i<MAX_ITEM;i++){item[i]=item[i+1];}
					}
				break;
				case BTN_UP:case BTN_IUP: // Move up
					item=(LOWORD(wp)==BTN_UP)?(port):(iwad);
					q=(LOWORD(wp)==BTN_UP)?(LST_PORT):(LST_IWAD);
					if((i=SendMessage(GetDlgItem(dlg,q),LB_GETCURSEL,0,0))!=LB_ERR&&i!=0){
						// Move the entry in the window
						SendMessage(GetDlgItem(dlg,q),LB_INSERTSTRING,i-1,(LPARAM)item[i]->name);
						SendMessage(GetDlgItem(dlg,q),LB_DELETESTRING,i+1,0);
						SendMessage(GetDlgItem(dlg,q),LB_SETCURSEL,i-1,0);
						// Move the item
						tmpi=item[i-1];item[i-1]=item[i];item[i]=tmpi;
					}
				break;
				case BTN_DWN:case BTN_IDWN: // Move down
					item=(LOWORD(wp)==BTN_DWN)?(port):(iwad);
					q=(LOWORD(wp)==BTN_DWN)?(LST_PORT):(LST_IWAD);
					i=SendMessage(GetDlgItem(dlg,q),LB_GETCURSEL,0,0);
					if((i=SendMessage(GetDlgItem(dlg,q),LB_GETCURSEL,0,0))!=LB_ERR&&i!=SendMessage(GetDlgItem(dlg,q),LB_GETCOUNT,0,0)-1){
						// Move the entry in the window
						SendMessage(GetDlgItem(dlg,q),LB_INSERTSTRING,i+2,(LPARAM)item[i]->name);
						SendMessage(GetDlgItem(dlg,q),LB_DELETESTRING,i,0);
						SendMessage(GetDlgItem(dlg,q),LB_SETCURSEL,i+1,0);
						// Move the item
						tmpi=item[i+1];item[i+1]=item[i];item[i]=tmpi;
					}
				break;
				case BTN_OK:
					SendMessage(GetDlgItem(dlg,EDT_EXTRA),WM_GETTEXT,sizeof(cfg.always),(LPARAM)cfg.always);
					cfg.launch=(char)SendMessage(GetDlgItem(dlg,CHK_LAUNCH),BM_GETCHECK,0,0);
					cfg.autoclose=(char)SendMessage(GetDlgItem(dlg,CHK_AUTOCLOSE),BM_GETCHECK,0,0);
					EndDialog(dlg,0);
				break;
			}break;
		}break;
	}return 0;
}

LRESULT CALLBACK MainProc(HWND dlg,UINT msg,WPARAM wp,LPARAM lp){
	int i=0,q=0,r=0,m=0;
	POINT pt;
	RECT rct;
	FILE *fptr=0;
	char *tmp=0,*tmp2=0,*label[3][9]={{"V. Easy","Easy","Normal","Hard","V. Hard"},{"Single-Player","Multi Co-operative","Multi DeathMatch"},{"Joining","1","2","3","4","5","6","7","8"}};
	OPENFILENAME ofn={sizeof(OPENFILENAME),dlg,GetModuleHandle(NULL),0,0,0,0,0,MAX_PATH,0,0,0,0,0,0,0,0,0,0,0};
	switch(msg){
		case WM_INITDIALOG:
		// Set icons
			SendMessage(dlg,WM_SETICON,ICON_SMALL,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_ICON)));
			SendMessage(dlg,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_ICON)));
			SendMessage(GetDlgItem(dlg,BTN_UP),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_UP)));
			SendMessage(GetDlgItem(dlg,BTN_DWN),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(ICO_DOWN)));
		// Insert list data
			for(i=0;i<5;i++){SendMessage(GetDlgItem(dlg,LST_SKILL),CB_ADDSTRING,0,(LPARAM)label[0][i]);}
			SendMessage(GetDlgItem(dlg,LST_SKILL),CB_SETCURSEL,2,0);
			for(i=0;i<3;i++){SendMessage(GetDlgItem(dlg,LST_GAME),CB_ADDSTRING,0,(LPARAM)label[1][i]);}
			SendMessage(GetDlgItem(dlg,LST_GAME),CB_SETCURSEL,0,0);
			for(i=0;i<9;i++){SendMessage(GetDlgItem(dlg,LST_PLAYERS),CB_ADDSTRING,0,(LPARAM)label[2][i]);}
			SendMessage(GetDlgItem(dlg,LST_PLAYERS),CB_SETCURSEL,0,0);
		// Set text limits
			SendMessage(GetDlgItem(dlg,LST_WARP),CB_LIMITTEXT,8,0);
			SendMessage(GetDlgItem(dlg,EDT_EXTRA),EM_LIMITTEXT,128,0);
			SendMessage(GetDlgItem(dlg,EDT_HOST),EM_LIMITTEXT,21,0);
			SendMessage(GetDlgItem(dlg,EDT_FRAGS),EM_LIMITTEXT,3,0);
			SendMessage(GetDlgItem(dlg,EDT_DMF),EM_LIMITTEXT,8,0);
			SendMessage(GetDlgItem(dlg,EDT_DMF2),EM_LIMITTEXT,8,0);
		// Fill the port and IWAD arrays
			if((i=Cfg_GetSel(0,port))!=-1){
				for(i;port[i]&&i<MAX_ITEM;i++){if(port[i]->avail){SendMessage(GetDlgItem(dlg,LST_PORT),CB_ADDSTRING,0,(LPARAM)port[i]->name);}}
				SendMessage(GetDlgItem(dlg,LST_PORT),CB_SETCURSEL,0,0);
			}
			if((i=Cfg_GetSel(0,iwad))!=-1){
				for(i;iwad[i]&&i<MAX_ITEM;i++){if(iwad[i]->avail){SendMessage(GetDlgItem(dlg,LST_IWAD),LB_ADDSTRING,0,(LPARAM)iwad[i]->name);}}
				SendMessage(GetDlgItem(dlg,LST_IWAD),LB_SETCURSEL,0,0);
				if(Cfg_GetSel(0,iwad)!=-1){Dlg_PopulateWarp(dlg,iwad[Cfg_GetSel(0,iwad)]->path);}
			}
		// Set the window size
			SendMessage(dlg,WM_COMMAND,MAKELONG(BTN_PANEL,BN_CLICKED),0);
		// Load the last configuration
			if(Cfg_GetSel(0,port)==-1||Cfg_GetSel(0,iwad)==-1){MessageBox(dlg,STR_NOITEMS,"Error!",MB_OK|MB_ICONEXCLAMATION);break;}
			if(!cmdline[0]){Cfg_ReadSave(dlg,cfg.ini);break;}
			// Process the command-line stuff
			memset((tmp=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
			for(i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCOUNT,0,0);cmdline[q];i++){
				if(i>=MAX_PWAD){MessageBox(dlg,STR_MAXPWAD,"Error!",MB_OK|MB_ICONEXCLAMATION);break;}
				for(q;cmdline[q];q++){
					if(cmdline[q]=='\"'){m=(m)?(0):(1);continue;}
					if(r<MAX_PATH){tmp[r]=cmdline[q];r++;}
					if(cmdline[q]==' '&&!m){q++;break;}
				}tmp[(cmdline[q])?(r-1):(r)]='\0';r=0;
				// Check to see if it's a save file and act accordingly.
				if(!Dlg_AddPWAD(dlg,tmp)){ // Check if it's a save file and launch
					if(i==0&&strchr(tmp,'.')&&(!stricmp(strchr(tmp,'.'),".zdl")||!stricmp(strchr(tmp,'.'),".ini"))){
						if(Cfg_ReadSave(dlg,tmp)){
							free(tmp);
							if(cfg.launch){ // Launch if the option is set
								Dlg_Launch(dlg,0);
								Dlg_Quit(dlg,0);
							}
						}else{
							free(tmp);
							if(cfg.launch){Dlg_Quit(dlg,0);}
						}
					}else{i--;}
				}
			}
		break;
		case WM_COMMAND:switch(HIWORD(wp)){
			// File Info
			case LBN_DBLCLK:switch(LOWORD(wp)){
				case LST_PWAD:MessageBox(dlg,pwad[SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCURSEL,0,0)],"File Info",MB_OK|MB_ICONINFORMATION);break;
				case LST_IWAD:MessageBox(dlg,iwad[Cfg_GetSel(SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCURSEL,0,0),iwad)]->path,"File Info",MB_OK|MB_ICONINFORMATION);break;
			}break;
			// IWAD List
			case LBN_SELCHANGE:if(LOWORD(wp)==LST_IWAD){Dlg_PopulateWarp(dlg,iwad[Cfg_GetSel(SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCURSEL,0,0),iwad)]->path);}break;
			// Buttons
			case BN_CLICKED:switch(LOWORD(wp)){
				case BTN_LAUNCH:Dlg_Launch(dlg,0);break;
				case MNU_CMD:Dlg_Launch(dlg,1);break;
				case BTN_ZDL:
					GetCursorPos(&pt);
					TrackPopupMenu(GetSubMenu(LoadMenu(GetModuleHandle(NULL),MAKEINTRESOURCE(MNU_MENU)),0),TPM_LEFTALIGN,pt.x,pt.y,0,dlg,0);
				break;
				// Loading and Saving
				case MNU_SAVE:case MNU_LOAD:
					memset((ofn.lpstrFile=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
					ofn.lpstrFilter="ZDL Save Files (*.zdl,*.ini)\0*.zdl;*.ini\0All Files (*.*)\0*.*\0";
					ofn.lpstrDefExt="zdl";
					ofn.Flags=(LOWORD(wp)==MNU_SAVE)?(OFN_EXPLORER):(OFN_FILEMUSTEXIST|OFN_EXPLORER);
					ofn.lpstrTitle=(LOWORD(wp)==MNU_SAVE)?("Save ZDL Config"):("Load a Saved ZDL Config");
					if(LOWORD(wp)==MNU_SAVE){
						if(GetSaveFileName(&ofn)&&(fptr=fopen(ofn.lpstrFile,"w"))){
							Cfg_WriteSave(dlg,fptr);
							fclose(fptr);
						}
					}else{if(GetOpenFileName(&ofn)){Cfg_ReadSave(dlg,ofn.lpstrFile);}}
					free(ofn.lpstrFile);
					break;
				case MNU_CLEAR:Dlg_ClearAll(dlg);break;
				case MNU_PWAD:Dlg_ClearPWAD(dlg);break;
				// Open and close the multiplay panel
				case BTN_PANEL:
					rct.top=0;rct.left=0;rct.right=270;rct.bottom=(cfg.dlgmode)?(282):(210);
					cfg.dlgmode=(cfg.dlgmode)?(0):(1);
					AdjustWindowRect(&rct,WS_POPUP|WS_CAPTION,0);
					SetWindowPos(dlg,0,0,0,rct.right-rct.left,rct.bottom-rct.top,SWP_NOMOVE|SWP_NOZORDER);
					SendMessage(GetDlgItem(dlg,BTN_PANEL),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE((cfg.dlgmode)?(ICO_DOWN):(ICO_UP))));
					if (cfg.dlgmode) {
						EnableWindow(GetDlgItem(dlg,LST_GAME), FALSE);
						EnableWindow(GetDlgItem(dlg,EDT_HOST), FALSE);
						EnableWindow(GetDlgItem(dlg,LST_PLAYERS), FALSE);
						EnableWindow(GetDlgItem(dlg,EDT_FRAGS), FALSE);
						EnableWindow(GetDlgItem(dlg,EDT_DMF), FALSE);
						EnableWindow(GetDlgItem(dlg,EDT_DMF2), FALSE);
					} else {
						EnableWindow(GetDlgItem(dlg,LST_GAME), TRUE);
						EnableWindow(GetDlgItem(dlg,EDT_HOST), TRUE);
						EnableWindow(GetDlgItem(dlg,LST_PLAYERS), TRUE);
						EnableWindow(GetDlgItem(dlg,EDT_FRAGS), TRUE);
						EnableWindow(GetDlgItem(dlg,EDT_DMF), TRUE);
						EnableWindow(GetDlgItem(dlg,EDT_DMF2), TRUE);
					}
					break;
				case BTN_ADD:if(pwad[MAX_PWAD-1]){MessageBox(dlg,"Too many PWADs loaded!","Error!",MB_OK|MB_ICONEXCLAMATION);break;}
					ofn.lpstrFilter="ZDoom Addon Files (*.wad,*.deh,*.bex,*.zip,*.pk3)\0*.wad;*.deh;*.bex;*.zip;*.pk3\0All Files (*.*)\0*.*\0";
					ofn.Flags=OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_ALLOWMULTISELECT;
					ofn.lpstrTitle="Load an External File";
					ofn.lpstrDefExt="wad";
					// Get the file and set the editbox
					memset((ofn.lpstrFile=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
					if(!GetOpenFileName(&ofn)){break;}
					i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCOUNT,0,0);
					if(!ofn.nFileExtension){ // Multiple files
						memset((tmp2=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
						tmp=ofn.lpstrFile+ofn.nFileOffset;
						for(i;i<MAX_PWAD&&tmp[0];i++){
							_snprintf(tmp2,MAX_PATH,"%s\\%s",ofn.lpstrFile,tmp);
							if(!Dlg_AddPWAD(dlg,tmp2)){i--;}
							tmp+=strlen(tmp)+1;
							if(i+1>=MAX_PWAD){MessageBox(dlg,STR_MAXPWAD,"Error!",MB_OK|MB_ICONEXCLAMATION);break;}
						}
						free(tmp2);
					}else{Dlg_AddPWAD(dlg,ofn.lpstrFile);}
					free(ofn.lpstrFile);
					break;
				case BTN_REM: // Delete button
					if((i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCURSEL,0,0))!=LB_ERR){
						free(pwad[i]);
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_DELETESTRING,i,0);
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_SETCURSEL,i,0);
						for(i;pwad[i]&&i<MAX_PWAD;i++){pwad[i]=pwad[i+1];}
					}
					break;
				case BTN_UP: // Move up
					if((i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCURSEL,0,0))!=LB_ERR&&i!=0){
						// Move the entry in the window
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_INSERTSTRING,i-1,(LPARAM)strrchr(pwad[i],'\\')+1);
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_DELETESTRING,i+1,0);
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_SETCURSEL,i-1,0);
						// Move the item
						tmp=pwad[i-1];pwad[i-1]=pwad[i];pwad[i]=tmp;
					}
					break;
				case BTN_DWN: // Move down
					i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCURSEL,0,0);
					if((i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCURSEL,0,0))!=LB_ERR&&i!=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCOUNT,0,0)-1){
						// Move the entry in the window
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_INSERTSTRING,i+2,(LPARAM)strrchr(pwad[i],'\\')+1);
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_DELETESTRING,i,0);
						SendMessage(GetDlgItem(dlg,LST_PWAD),LB_SETCURSEL,i+1,0);
						// Move the item
						tmp=pwad[i+1];pwad[i+1]=pwad[i];pwad[i]=tmp;
					}
					break;
				// Menu Selections
				case MNU_OPTIONS:
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_OPTIONS),dlg,ConfigProc);
					// Fill the port and IWAD arrays
					SendMessage(GetDlgItem(dlg,LST_PORT),CB_RESETCONTENT,0,0);
					for(i=0;port[i]&&i<MAX_ITEM;i++){if(port[i]->avail){SendMessage(GetDlgItem(dlg,LST_PORT),CB_ADDSTRING,0,(LPARAM)port[i]->name);}}
					SendMessage(GetDlgItem(dlg,LST_PORT),CB_SETCURSEL,0,0);
					SendMessage(GetDlgItem(dlg,LST_IWAD),LB_RESETCONTENT,0,0);
					for(i=0;iwad[i]&&i<MAX_ITEM;i++){if(iwad[i]->avail){SendMessage(GetDlgItem(dlg,LST_IWAD),LB_ADDSTRING,0,(LPARAM)iwad[i]->name);}}
					SendMessage(GetDlgItem(dlg,LST_IWAD),LB_SETCURSEL,0,0);
					SendMessage(GetDlgItem(dlg,LST_WARP),CB_RESETCONTENT,0,0);
					if(SendMessage(GetDlgItem(dlg,LST_IWAD),LB_GETCURSEL,0,0)!=LB_ERR){Dlg_PopulateWarp(dlg,iwad[Cfg_GetSel(0,iwad)]->path);}
				break;
				case MNU_ABOUT:
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(DLG_ABOUT),dlg,AboutProc);
					break;
				// Ways to exit
				case BTN_EXIT:
					SendMessage(dlg,WM_CLOSE,0,0);
					break;
			}break;
		}break;
		case WM_DROPFILES: // Add files dropped onto the pwad box
			r=DragQueryFile((HDROP)wp,0xFFFFFFFF,0,0);
			memset((tmp2=malloc(MAX_PATH*sizeof(char))),0,MAX_PATH*sizeof(char));
			for(i=SendMessage(GetDlgItem(dlg,LST_PWAD),LB_GETCOUNT,0,0);q<r;i++){
				if(i>=MAX_PWAD){MessageBox(dlg,STR_MAXPWAD,"Error!",MB_OK|MB_ICONEXCLAMATION);break;}
				DragQueryFile((HDROP)wp,q,tmp2,MAX_PATH);
				if(!Dlg_AddPWAD(dlg,tmp2)){i--;}
			q++;}
			DragFinish((HDROP)wp);
			free(tmp2);
		break;
		case WM_CLOSE:Dlg_Quit(dlg,1);break;
	}return 0;
}

int WINAPI WinMain(HINSTANCE inst,HINSTANCE pinst,LPSTR cline,int cshow){
	// Set up the INI string
	cfg.ini = calloc( strlen(_pgmptr)+8 , sizeof(char) );
	strcpy(cfg.ini,_pgmptr);
	strrchr(cfg.ini,'\\')[1]='\0';
	strcat(cfg.ini,"zdl.ini");
	// We need the command line for the INI thingy later
	cmdline=cline;
	Cfg_LoadConfig(); // Load the configuration file
	// GoooOOooooOOOOOoooOOO!
	return DialogBox(inst,MAKEINTRESOURCE(DLG_MAIN),HWND_DESKTOP,MainProc);
}
