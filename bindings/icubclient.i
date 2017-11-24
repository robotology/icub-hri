%module icubclient

%include <stl.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
%include <std_except.i>

%{
#include <icubclient/tags.h>
#include <icubclient/functions.h>

#include <icubclient/knowledge/entity.h>
#include <icubclient/knowledge/object.h>
#include <icubclient/knowledge/action.h>
#include <icubclient/knowledge/relation.h>
#include <icubclient/knowledge/agent.h>

#include <icubclient/subsystems/subSystem.h>
#include <icubclient/subsystems/subSystem_agentDetector.h>
#include <icubclient/subsystems/subSystem_ARE.h>
#include <icubclient/subsystems/subSystem_babbling.h>
#include <icubclient/subsystems/subSystem_iol2opc.h>
#include <icubclient/subsystems/subSystem_KARMA.h>
#include <icubclient/subsystems/subSystem_recog.h>
#include <icubclient/subsystems/subSystem_speech.h>
#include <icubclient/subsystems/subSystem_SAM.h>
#include <icubclient/subsystems/subSystem_emotion.h>

#include <icubclient/clients/opcClient.h>
#include <icubclient/clients/icubClient.h>

%}

%include <icubclient/tags.h>
%include <icubclient/functions.h>

%include <icubclient/knowledge/entity.h>
%include <icubclient/knowledge/object.h>
%include <icubclient/knowledge/action.h>
%include <icubclient/knowledge/relation.h>
%include <icubclient/knowledge/agent.h>

%include <icubclient/subsystems/subSystem.h>
%include <icubclient/subsystems/subSystem_agentDetector.h>
%include <icubclient/subsystems/subSystem_ARE.h>
%include <icubclient/subsystems/subSystem_babbling.h>
%include <icubclient/subsystems/subSystem_iol2opc.h>
%include <icubclient/subsystems/subSystem_KARMA.h>
%include <icubclient/subsystems/subSystem_recog.h>
%include <icubclient/subsystems/subSystem_speech.h>
%include <icubclient/subsystems/subSystem_SAM.h>
%include <icubclient/subsystems/subSystem_emotion.h>

%include <icubclient/clients/opcClient.h>
%include <icubclient/clients/icubClient.h>

