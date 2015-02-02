#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>



class Debugger_base : public Place
{
 private:
  class SocketGUIConnection : public Thread
  {
  private:
    Debugger_base *const outerInstance;

    ObjectOutputStream *output;
    ObjectInputStream *input;

  public:
    SocketGUIConnection(Debugger_base *outerInstance, ObjectOutputStream *output, ObjectInputStream *input);

    virtual void run() override;

  private:
    void probeGUICmd();

    //private void 
    void processCommand(const std::wstring &cmd);

  };


 private:
  int placesHandler = 0;
  int agentsHandler = 0;
  Places_base *places;
  Places_base *debugger_places;
  Agents_base *agents;
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: private int[] psize;
  int *psize;

  //JAVA TO C++ CONVERTER TODO TASK: Most Java annotations will not have direct C++ equivalents:
  //ORIGINAL LINE: @SuppressWarnings("unused") private int nDimen;
  int nDimen = 0;

  int pid = 0;
  int nTotalPlace = 0; //total user places
  int debugger_size = 0; //total number of debugger place
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: private Object[] placeDebugData;
  void **placeDebugData;
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: private Object[] agentDebugData;
  void **agentDebugData;
  std::vector<AgentDebugData*> tmp_agentDebugData;
  static int dataConnectionThreadstatus = 0;
  bool isPlaceAgentMode = false; //true: place+agent, false: place
 public:
  static Debugger_base *debuggerInstance;

  /*public final static int init_ = 0;
      public final static int fetchDebugData_ = 1;
      public final static int injectDebugData_ = 2;*/
 private:
  static const int SOCKET_PORT = 40863;

  //JAVA TO C++ CONVERTER TODO TASK: Most Java annotations will not have direct C++ equivalents:
  //ORIGINAL LINE: @SuppressWarnings("unused") private final static String CMD_NEW_APPLICATION = "cmdNewApplication";
  static const std::wstring CMD_NEW_APPLICATION;

  //JAVA TO C++ CONVERTER TODO TASK: Most Java annotations will not have direct C++ equivalents:
  //ORIGINAL LINE: @SuppressWarnings("unused") private final static String CMD_PLACE_DATA = "cmdPlaceData";
  static const std::wstring CMD_PLACE_DATA;

  //JAVA TO C++ CONVERTER TODO TASK: Most Java annotations will not have direct C++ equivalents:
  //ORIGINAL LINE: @SuppressWarnings("unused") private final static String CMD_AGENT_DATA = "cmdAgentData";
  static const std::wstring CMD_AGENT_DATA;

  static const std::wstring CMD_PAUSE;
  static const std::wstring CMD_RESUME;
  static const std::wstring CMD_INJECT_PLACE;
 protected:
  static const int STATUS_READY = 0;
  static const int STATUS_SEND_PLACE_DATA = 1;
  static const int STATUS_SEND_AGENT_DATA = 2;

  /*
   * if sending_lock is true means debugger is sending data to GUI
   * if stop_lock is true means user has stopped the computation
   */
 public:
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: public static boolean[] sending_lock;
  static bool *sending_lock;
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: public static boolean[] stop_lock;
  static bool *stop_lock;
 private:
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: private static boolean[] sending_place_lock;
  static bool *sending_place_lock;



  /// <param name="places"> is the Places handler, agents is the Agents handler
  ///  </param>
 public:
  Debugger_base(void *argument);

 protected:
  virtual void *init(void *args);

  /*public Object callMethod(int functionId, Object argument) {
      switch (functionId) {
        case init_: 
	case fetchDebugData_: return fetchDebugData(argument);
	case injectDebugData_: return InjectDebugData(argument);
	default: break;
      }
      return null;
  }*/

  virtual void *fetchDebugData(void *argument);

  virtual void *fetchAgentDebugData(void *argument);

  virtual void *InjectDebugData(void *argument);
  
  virtual void *injectAgentDebugData(void *argument);

  static void updateDataConnectionThread(int status);

 private:
  void startTCPSocketServer();

 private:
  class ThreadAnonymousInnerClassHelper : public Thread
  {
  private:
    Debugger_base *const outerInstance;

  public:
    ThreadAnonymousInnerClassHelper(Debugger_base *outerInstance);

    virtual void run() override;
  };

};

class SinglePlaceAgentData : public Serializable
{

 private:
  static const long long serialVersionUID = 1LL;
  int x = 0;
  int y = 0;
  int place_val = 0;
  std::vector<int> agents;

 public:
  //JAVA TO C++ CONVERTER TODO TASK: Most Java annotations will not have direct C++ equivalents:
  //ORIGINAL LINE: @SuppressWarnings("unchecked") public SinglePlaceAgentData(int x, int y, int val, java.util.ArrayList<Integer> agents)
  SinglePlaceAgentData(int x, int y, int val, std::vector<int> &agents);
  virtual int getX();
  virtual void setX(int x);
  virtual int getY();
  virtual void setY(int y);
  virtual int getPlace_val();
  virtual void setPlace_val(int place_val);
  virtual std::vector<int> getAgents();
  virtual void setAgents(std::vector<int> &agents);
};

class AgentDebugData : public Serializable
{

 private:
  static const long long serialVersionUID = 1LL;
 public:
  int x = 0;
  int y = 0;
  double value = 0;

  AgentDebugData(int x, int y, double value);

};
