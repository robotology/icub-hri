%module icubhri

%include <stl.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>
%include <std_except.i>

%{
#include <icubhri/tags.h>
#include <icubhri/functions.h>

#include <icubhri/knowledge/entity.h>
#include <icubhri/knowledge/object.h>
#include <icubhri/knowledge/action.h>
#include <icubhri/knowledge/relation.h>
#include <icubhri/knowledge/agent.h>

#include <icubhri/subsystems/subSystem.h>
#include <icubhri/subsystems/subSystem_agentDetector.h>
#include <icubhri/subsystems/subSystem_ARE.h>
#include <icubhri/subsystems/subSystem_babbling.h>
#include <icubhri/subsystems/subSystem_iol2opc.h>
#include <icubhri/subsystems/subSystem_KARMA.h>
#include <icubhri/subsystems/subSystem_recog.h>
#include <icubhri/subsystems/subSystem_speech.h>
#include <icubhri/subsystems/subSystem_SAM.h>

#include <icubhri/clients/opcClient.h>
#include <icubhri/clients/icubClient.h>

%}

%include <icubhri/tags.h>
%include <icubhri/functions.h>

%include <icubhri/knowledge/entity.h>
%include <icubhri/knowledge/object.h>
%include <icubhri/knowledge/action.h>
%include <icubhri/knowledge/relation.h>
%include <icubhri/knowledge/agent.h>

%include <icubhri/subsystems/subSystem.h>
%include <icubhri/subsystems/subSystem_agentDetector.h>
%include <icubhri/subsystems/subSystem_ARE.h>
%include <icubhri/subsystems/subSystem_babbling.h>
%include <icubhri/subsystems/subSystem_iol2opc.h>
%include <icubhri/subsystems/subSystem_KARMA.h>
%include <icubhri/subsystems/subSystem_recog.h>
%include <icubhri/subsystems/subSystem_speech.h>
%include <icubhri/subsystems/subSystem_SAM.h>

%include <icubhri/clients/opcClient.h>
%include <icubhri/clients/icubClient.h>

