// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_iol2opc_IDL
#define YARP_THRIFT_GENERATOR_iol2opc_IDL

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class iol2opc_IDL;


/**
 * iol2opc_IDL
 * IDL Interface to \ref iol2opc service.
 */
class iol2opc_IDL : public yarp::os::Wire {
public:
  iol2opc_IDL();
  /**
   * Provide the object name of the previously selected blob.
   * @param name is the object name
   * @return true/false on success/failure.
   */
  virtual bool assignName(const std::string& name);
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

