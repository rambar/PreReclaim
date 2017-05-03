#include <iostream>
#include <vector>
#include <stdlib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "loader.h"

using namespace std;	

#define CONFIGURATION_LAUNCHTYPE	"launchType"
#define CONFIGURATION_COMMAND 		"command"
#define CONFIGURATION_MONITOR 		"monitor"
#define CONFIGURATION_USAGEBELOW	"usageBelow"

#define PROC_EFM		"/proc/sys/vm/extra_free_kbytes"
#define SIZE_TO_RECLAIM "150000"
#define SIZE_ZERO		"0"

#define FORK_EXEC(...) 	AddPreJob(testset); \
						testset.AddForkAndExec(__VA_ARGS__); \
						AddPostJob(testset);

#define AUL_LAUNCH(...) AddPreJob(testset); \
						testset.AddAulLaunch(__VA_ARGS__); \
						AddPostJob(testset);

void AddPreJob(TestSet &testset) {
	if(testset.PreReclaimEnabled()) {
		testset.AddProcWrite(PROC_EFM, SIZE_TO_RECLAIM);
		testset.AddSleep(3000);
		testset.AddProcWrite(PROC_EFM, SIZE_ZERO);
	}
}

void AddPostJob(TestSet &testset) {
	testset.AddSleep(5000);
}

bool Loader::LoadFromFile(string &filename, TestSet &testset) {
	JsonParser *parser;
	JsonNode *root;
	GError *error;
	JsonArray* array;
	guint arraySize;
	bool returnValue = false;

	parser = json_parser_new ();

	error = NULL;
	json_parser_load_from_file (parser, filename.c_str(), &error);
	if (error) {
		cout << "Unable to parse:" << error->message << endl; 
		g_error_free (error);
		goto finish;
	}

	root = json_parser_get_root (parser);

	if (root == NULL || (JSON_NODE_TYPE (root) != JSON_NODE_ARRAY))
		goto finish;
	
	/*
	cout << "JSON_NODE_TYPE(root) :" << JSON_NODE_TYPE(root) << endl;
	cout << "JSON_NODE_ARRAY :" << JSON_NODE_ARRAY << endl;
	cout << "JSON_NODE_OBJECT :" << JSON_NODE_OBJECT << endl;
	*/
	
	array = json_node_get_array(root);
	arraySize = json_array_get_length (array);

	if(arraySize <= 0) 
		goto finish;
    
	for(guint i = 0; i < arraySize; i++) {
		string 				value;
		TestSet::LaunchType launchType;
		string 				command;
		TestSet::MonitorType monitor = TestSet::S_MONITOR_CPU_TOTAL;
		double 				usageBelow = 50.0f;
		
		JsonNode *arrayElem = json_array_get_element (array, i);
		JsonObject *object  = json_node_get_object (arrayElem);

		if(object == NULL) goto finish;

		value = json_object_get_string_member (object, CONFIGURATION_LAUNCHTYPE);		

		if(!value.compare("ForkAndExec")) 
			launchType = TestSet::S_LAUNCH_FORK_AND_EXEC;
		else if(!value.compare("QuickCommand"))
			launchType = TestSet::S_LAUNCH_QUICK_COMMAND;
		else if(!value.compare("AulLaunch"))
			launchType = TestSet::S_LAUNCH_AUL_LAUNCH;
		else if(!value.compare("Sleep"))
			launchType = TestSet::S_LAUNCH_SLEEP;
		else if(!value.compare("ProcWrite"))
			launchType = TestSet::S_LAUNCH_PROC_WRITE;

		if(json_object_has_member (object, CONFIGURATION_COMMAND)) {
			command = json_object_get_string_member (object, CONFIGURATION_COMMAND);
		}

		if(json_object_has_member (object, CONFIGURATION_MONITOR)) {
			value = json_object_get_string_member (object, CONFIGURATION_MONITOR);

			if(!value.compare("CPU_TOTAL"))
				monitor = TestSet::S_MONITOR_CPU_TOTAL;
			else if(!value.compare("USER_PROC"))
				monitor = TestSet::S_MONITOR_USER_PROC;
		}

		if(json_object_has_member (object, CONFIGURATION_USAGEBELOW)) {
			usageBelow = json_object_get_double_member (object, CONFIGURATION_USAGEBELOW);
		}

		if(launchType == TestSet::S_LAUNCH_FORK_AND_EXEC){
			FORK_EXEC(command, monitor, usageBelow);
		}
		else if(launchType == TestSet::S_LAUNCH_AUL_LAUNCH){
			AUL_LAUNCH(command, monitor, usageBelow);
		}
	}

	returnValue = true;

finish:
	g_object_unref (parser);

	return returnValue;
}
