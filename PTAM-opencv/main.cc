// Copyright 2008 Isis Innovation Limited
// This is the main extry point for PTAM
#include <stdlib.h>
#include <iostream>
#include <gvars3/instances.h>
#include "System.h"


using namespace std;
using namespace GVars3;

int main(int argc, char **argv)
{
  cout << "  Welcome to PTAM " << endl;
  cout << "  --------------- " << endl;
  cout << "  Parallel tracking and mapping for Small AR workspaces" << endl;
  cout << "  Copyright (C) Isis Innovation Limited 2008 " << endl;  
  cout << endl;
  cout << "  Parsing settings.cfg ...." << endl;
  GUI.LoadFile("settings.cfg");
  
  GUI.StartParserThread(); // Start parsing of the console input
  atexit(GUI.StopParserThread); 

	int index_1;
	int index_2;

	if ( argc != 3 ) /* argc should be 2 for correct execution */
	{
		/* we print argv[0] assuming it is the program name */
		printf( "usage: ptam.exe [camera index 0] [camera index 1], \n example: ptam.exe 0 1\n please, do not use nagative index");
		system("pause");
		return 0;
		//index_1 = 1;
		//index_2 = 0;
	}
	else
	{
		index_1 = atoi(argv[1]);
		index_2 = atoi(argv[2]);
		cout<<"index :"<<index_1<<"  "<<index_2<<endl;
	}
  
  try
    {
      System s(index_1, index_2);
      s.Run();
    }
  catch(CVD::Exceptions::All e)
    {
      cout << endl;
      cout << "!! Failed to run system; got exception. " << endl;
      cout << "   Exception was: " << endl;
      cout << e.what << endl;
    }
}










