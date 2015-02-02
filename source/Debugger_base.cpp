#include "Debugger_base.h"



Debugger_base::SocketGUIConnection::SocketGUIConnection(Debugger_base *outerInstance, ObjectOutputStream *output, ObjectInputStream *input) : outerInstance(outerInstance)
{
  this->output = output;
  this->input = input;
}

void Debugger_base::SocketGUIConnection::run()
{

  // send array dimension and program mode (PLACE_ONLY, PLACE_AGENT)
  try
    {
      output->writeObject(L"cmdNewApplication");
      output->writeObject(outerInstance->places->places[0].getClass().getName());
      output->writeInt((outerInstance->psize[0])); //for simple just send the size[0], currently assume its N*N array
      output->writeBoolean(outerInstance->isPlaceAgentMode); //send the mode of user application
      output->flush();
    }
  catch (IOException e)
    {
      e->printStackTrace();
      return;
    }

  while (true)
    {
      try
	{
	  delay(1000);
	}
      catch (std::exception &e)
	{
	  std::cout << std::wstring(L"exception in sleep") << std::endl;
	}

      std::cout << std::wstring(L"status:") << dataConnectionThreadstatus << std::endl;
      //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
      synchronized(debuggerInstance)
      {
	probeGUICmd();
	switch (dataConnectionThreadstatus)
	  {
	  case STATUS_SEND_PLACE_DATA:
	    //dataConnectionThreadstatus = STATUS_READY;
	    //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
	    synchronized(sending_lock)
	    {
	      if (!sending_lock[0])
		{
		  std::cout << std::wstring(L"sending lock is false") << std::endl;
		  break;
		}
	      try
		{
		  output->writeObject(L"cmdPlaceData");

		  for (int i = 0; i < MASS_base::currentReturns::length; i++)
		    {
		      //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
		      //ORIGINAL LINE: Double[] d = (Double[])(MASS_base.currentReturns[i]);
		      double *d = static_cast<double[]>(MASS_base::currentReturns[i]);
		      for (int j = 0; j < d->length; j++)
			{
			  output->writeDouble(d[j]);
			}
		    }
		  output->flush();


		}
	      catch (IOException ioe)
		{
		  //ioe.printStackTrace();
		}
	      //JAVA TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
	      finally
		{

		}
	      //clear sending_lock if user applications is only place based
	      if (!outerInstance->isPlaceAgentMode)
		{
		  sending_lock[0] = false;
		  sending_lock->notifyAll();
		}
	    }
	    //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
	    synchronized(sending_place_lock)
	    {
	      sending_place_lock[0] = false;
	      sending_place_lock->notify();
	    }
	    break;
	  case STATUS_SEND_AGENT_DATA:
	    //dataConnectionThreadstatus = STATUS_READY;
	    //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
	    synchronized(sending_lock)
	    {
	      if (!sending_lock[0])
		{
		  break;
		}
	      try
		{
		  output->writeObject(L"cmdAgentData");
		  int nAgents = 0;
		  for (int i = 0; i < MASS_base::currentReturns::length; i++)
		    {
		      nAgents += (static_cast<AgentDebugData*[]>(MASS_base::currentReturns[i]))->length;
		    }
		  //send the number of agents
		  //JAVA TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'toString':
		  output->writeObject(int::toString(nAgents));
		  for (int i = 0; i < MASS_base::currentReturns::length; i++)
		    {
		      //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
		      //ORIGINAL LINE: AgentDebugData[] next_agents= (AgentDebugData[])(MASS_base.currentReturns[i]);
		      AgentDebugData **next_agents = static_cast<AgentDebugData*[]>(MASS_base::currentReturns[i]);
		      for (int j = 0; j < next_agents->length; j++)
			{
			  output->writeObject(next_agents[j]);
			}
		    }
		  output->flush();
		}
	      catch (IOException ioe)
		{

		}
	      //JAVA TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
	      finally
		{
		}
	      sending_lock[0] = false;
	      sending_lock->notifyAll();
	    }
	  case STATUS_READY:
	    break;
	  default:
	    break;
	  }
      }
      //Debugger_base.updateDataConnectionThread(STATUS_READY);
    }
}

void Debugger_base::SocketGUIConnection::probeGUICmd()
{
  try
    {
      //if(input.available() <= 0) return;
      std::wstring cmd = L"";
      cmd = static_cast<std::wstring>(input->readObject()); //block 100ms
      std::cout << std::wstring(L"Received:") << cmd << std::endl;
      if (cmd != L"" && cmd.length() > 0)
	{
	  processCommand(cmd);
	}
    }
  catch (IOException e)
    {
      //System.out.println("no data from GUI");
    }
  catch (ClassNotFoundException e)
    {
      e->printStackTrace();
    }
  //JAVA TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
  finally
    {

    }
}

void Debugger_base::SocketGUIConnection::processCommand(const std::wstring &cmd)
{
  if (cmd == CMD_PAUSE)
    { //user stop the computation
      //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
      synchronized(stop_lock)
      {
	stop_lock[0] = true;
      }
    }
  else if (cmd == CMD_RESUME)
    { //user resume the computation
      //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
      synchronized(stop_lock)
      {
	stop_lock[0] = false;
	stop_lock->notifyAll();
      }
    }
  else if (cmd == CMD_INJECT_PLACE)
    {
      try
	{
	  //SinglePlaceAgentData spaData = (SinglePlaceAgentData)input.readObject();
	  int x = std::stoi(static_cast<std::wstring>(input->readObject()));
	  int y = std::stoi(static_cast<std::wstring>(input->readObject()));
	  int v = std::stoi(static_cast<std::wstring>(input->readObject()));
	  //System.out.println("x:"+x+"y:"+y+"v:"+v);
	  SinglePlaceAgentData *data = new SinglePlaceAgentData(x,y,v,nullptr);
	  outerInstance->debugger_places->callAll(2, data, 0);
	}
      catch (IOException e)
	{
	  e->printStackTrace();
	}
      catch (ClassNotFoundException e)
	{
	  e->printStackTrace();
	}
    }
}

const std::wstring Debugger_base::CMD_NEW_APPLICATION = L"cmdNewApplication";
const std::wstring Debugger_base::CMD_PLACE_DATA = L"cmdPlaceData";
const std::wstring Debugger_base::CMD_AGENT_DATA = L"cmdAgentData";
const std::wstring Debugger_base::CMD_PAUSE = L"cmdPause";
const std::wstring Debugger_base::CMD_RESUME = L"cmdResume";
const std::wstring Debugger_base::CMD_INJECT_PLACE = L"cmdInjectPlace";

Debugger_base::Debugger_base(void *argument)
{
  //JAVA TO C++ CONVERTER WARNING: Java to C++ Converter has converted this array to a pointer. You will need to call 'delete[]' where appropriate:
  //ORIGINAL LINE: int[] handler = (int[]) argument;
  int *handler = static_cast<int[]>(argument);
  debuggerInstance = this;
  dataConnectionThreadstatus = STATUS_READY;
  this->placesHandler = handler[0];
  this->agentsHandler = handler[1];
}

void *Debugger_base::init(void *args)
{
  sending_lock = new bool[1] ();
  sending_lock[0] = false;
  stop_lock = new bool[1] ();
  stop_lock[0] = false;
  sending_place_lock = new bool[1] ();
  sending_place_lock[0] = false;

  debugger_size = size[0];
  places = MASS_base::placesMap->get(placesHandler);
  agents = MASS_base::agentsMap->get(agentsHandler);
  isPlaceAgentMode = agents == nullptr ? false : true;
  debugger_places = MASS_base::placesMap->get(99);
  pid = MASS_base::getMyPid();
  psize = places->size.clone();

  nTotalPlace = 1;
  for (int i = 0; i < psize->length; i++)
    {
      nTotalPlace *= psize[i];
    }

  nDimen = psize->length;
  placeDebugData = new double[places->places_size] ();
  tmp_agentDebugData = std::vector<AgentDebugData*>();

  // start a TCP socket server to communicate with GUI
  if (pid == 0)
    {
      std::cout << std::wstring(L"System size: ") << debugger_size << std::endl;
      std::cout << std::wstring(L"Total number of place: ") << nTotalPlace << std::endl;
      std::cout << std::wstring(L"places size ") << places->places_size << std::endl;
      std::cout << std::wstring(L"places array size ") << places->places.length << std::endl;
      std::cout << std::wstring(L"place name ") << places->places[0].getClass().getName() << std::endl;

      startTCPSocketServer();
    }
  return nullptr;
}

void *Debugger_base::fetchDebugData(void *argument)
{
  // new Object[MASS_base.currentPlaces.places_size];
  tmp_agentDebugData.clear();
  for (int i = 0; i < places->places_size; i++)
    {
      Place *curPlace = places->places[i];
      placeDebugData[i] = static_cast<double>(curPlace->getDebugData());
      //get agent debug data
      //    for(int j=0; j<curPlace.agents.size(); j++){
      for (Agent *agent : curPlace->agents)
	{

	  if (pid == 0)
	    {
	      //System.out.println(curPlace.agents.get(j).agentId);
	    }

	  AgentDebugData *tmp = static_cast<AgentDebugData*>(agent.getDebugData());
	  if (tmp != nullptr)
	    {
	      tmp_agentDebugData.push_back(tmp);
	    }

	}

      if (pid != 0 && (static_cast<double>(placeDebugData[i])) != 0 && (static_cast<double>(placeDebugData[i])) - 20.0 != 0)
	{
	  //MASS_base.log(String.valueOf(((Double)placeDebugData[i]).doubleValue()));
	}
    }

  return static_cast<void*>(placeDebugData);
}

void *Debugger_base::fetchAgentDebugData(void *argument)
{
  int n = tmp_agentDebugData.size();
  agentDebugData = new AgentDebugData*[n] ();
  for (int i = 0; i < n; i++)
    {
      AgentDebugData *a = tmp_agentDebugData[i];
      agentDebugData[i] = new AgentDebugData(a->x, a->y, a->value);
    }
  /*
    int nAgents = ((Agents)agents).nAgents();
    agentDebugData = new AgentDebugData[nAgents];
    int i = Mthread.agentBagSize, j=0;
    if(pid==0) System.out.println("agent bag size: "+i);
    for(; i > 0; i--){
        agentDebugData[j++] = agents.agents.get(i).getDebugData();
	    if(pid == 0){
	    System.out.println(((AgentDebugData)agentDebugData[j-1]).x+
	       " "+((AgentDebugData)agentDebugData[j-1]).y+
	          " "+((AgentDebugData)agentDebugData[j-1]).value);
		      }
		      }*/
  return static_cast<void*>(agentDebugData);

}

void *Debugger_base::InjectDebugData(void *argument)
{
  //debugger places are one dimention
  SinglePlaceAgentData *spaData = static_cast<SinglePlaceAgentData*>(argument);
  int x = spaData->getX();
  int y = spaData->getY();
  int place_val = spaData->getPlace_val();

  int stripe = nTotalPlace / debugger_size;
  int offset = (x*psize[0] + y) / stripe;
  if (offset != pid)
    {
      return nullptr;
    }
  //check if x and y belong to this node
  int i = (x*psize[0] + y) - stripe*pid;
  places->places[i].setDebugData(double(place_val));
  return nullptr;
}

void *Debugger_base::injectAgentDebugData(void *argument)
{
  //the argument contains place index and agent index
  return nullptr;
}

void Debugger_base::updateDataConnectionThread(int status)
{
  if (status == STATUS_SEND_PLACE_DATA)
    {
      //System.out.println("STATUS_SEND_PLACE_DATA");
      //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
      synchronized(sending_place_lock)
      {
	sending_place_lock[0] = true;
      }
      //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
      synchronized(sending_lock)
      {
	sending_lock[0] = true;
      }
    }
  else if (status == STATUS_SEND_AGENT_DATA)
    {
      //System.out.println("STATUS_SEND_AGENT_DATA");
      //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
      synchronized(sending_place_lock)
      {
	if (sending_place_lock[0])
	  {
	    std::cout << std::wstring(L"pending to send agent data") << std::endl;
	    try
	      {
		sending_place_lock->wait();
	      }
	    catch (std::exception &e)
	      {
		std::cout << std::wstring(L"wait failed") << std::endl;
	      }
	  }
      }
    }
  else
    {

    }
  //JAVA TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
  synchronized(debuggerInstance)
  {
    dataConnectionThreadstatus = status;
  }
}

void Debugger_base::startTCPSocketServer()
{

  new ThreadAnonymousInnerClassHelper(this)
    .start();
}

Debugger_base::ThreadAnonymousInnerClassHelper::ThreadAnonymousInnerClassHelper(Debugger_base *outerInstance)
{
  this->outerInstance = outerInstance;
}

void Debugger_base::ThreadAnonymousInnerClassHelper::run()
{
  ObjectInputStream *input = nullptr;
  ObjectOutputStream *output = nullptr;
  ServerSocket *server = nullptr;
  Socket *guiClient = nullptr;
  try
    {
      server = new ServerSocket(SOCKET_PORT);
    }
  catch (IOException e)
    {
      e->printStackTrace();
    }
  while (true)
    {
      try
	{
	  guiClient = server->accept();
	}
      catch (SocketTimeoutException ste)
	{

	}
      catch (IOException e)
	{
	  e->printStackTrace();
	}

      if (guiClient != nullptr)
	{
	  break;
	}
    }

  try
    {
      std::cout << guiClient->getRemoteSocketAddress() << std::endl;
      std::cout << std::wstring(L"local socket address: ") << guiClient->getLocalSocketAddress() << std::endl;
      guiClient->setSoTimeout(100);
      output = new ObjectOutputStream(guiClient->getOutputStream());
      output->flush();
      input = new ObjectInputStream(guiClient->getInputStream());
    }
  catch (IOException e)
    {
      e->printStackTrace();
    }

  SocketGUIConnection *guiConnection = new SocketGUIConnection(outerInstance, output, input);
  guiConnection->start();
}

//JAVA TO C++ CONVERTER TODO TASK: Most Java annotations will not have direct C++ equivalents:
//ORIGINAL LINE: @SuppressWarnings("unchecked") public SinglePlaceAgentData(int x, int y, int val, java.util.ArrayList<Integer> agents)
SinglePlaceAgentData::SinglePlaceAgentData(int x, int y, int val, std::vector<int> &agents)
{

  this->x = x;
  this->y = y;
  this->place_val = val;
  this->agents = std::vector<int>();
  if (agents.size() > 0)
    {
      this->agents = static_cast<std::vector<int>>(agents.clone());
    }
}

int SinglePlaceAgentData::getX()
{
  return x;
}

void SinglePlaceAgentData::setX(int x)
{
  this->x = x;
}

int SinglePlaceAgentData::getY()
{
  return y;
}

void SinglePlaceAgentData::setY(int y)
{
  this->y = y;
}

int SinglePlaceAgentData::getPlace_val()
{
  return place_val;
}

void SinglePlaceAgentData::setPlace_val(int place_val)
{
  this->place_val = place_val;
}

std::vector<int> SinglePlaceAgentData::getAgents()
{
  return agents;
}

void SinglePlaceAgentData::setAgents(std::vector<int> &agents)
{
  this->agents = agents;
}

AgentDebugData::AgentDebugData(int x, int y, double value)
{
  this->x = x;
  this->y = y;
  this->value = value;
}
