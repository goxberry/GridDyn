/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef GRID_SIMULATION_H_
#define GRID_SIMULATION_H_

// libraries
#include <list>
#include <fstream>

// header files
#include "gridArea.h"

class gridLink;
class gridBus;


#define SINGLE (1)
#define MULTICORE       (2)
#define MULTIPROCESSOR (3)


//define ERROR codes for the error state
#define GS_NO_ERROR 0
#define GS_FILE_LOAD_ERROR (-1)
#define GS_INVALID_FILE_ERROR (-2)

//define PRINT LEVELS

class gridRecorder;
class gridEvent;
class gridRelay;
class eventQueue;
class eventAdapter;
class functionEventAdapter;
/** @brief Grid simulation object
 GridSimulation is a base simulation class its intention is to handle some of the basic
simulation bookkeeping tasks that would be common to all simulations in Griddyn including things like
logging, errors,  recording, and a few other topics.  Doesn't really do anything much as far as simulating goes
the class isn't abstract but it is intended to be built upon
*/
class gridSimulation : public gridArea
{
public:
  std::string sourceFile;                                            //!<main source file name
  /** @brief enumeration describing the state of the griddyn simulation*/
  enum class gridState_t : int
  {
    HALTED = -2,             //!< the system is halted for some reason
    GD_ERROR = -1,                      //!< the system has an error
    STARTUP = 0,                        //!< the system is starting up either loaded or unloaded
    INITIALIZED = 1,                    //!< the system has completed a power flow initialization step and is ready to proceed to power flow calculations
    POWERFLOW_COMPLETE = 2,             //!< the sytem has completed a power flow calculation successfully
    DYNAMIC_INITIALIZED = 3,                    //!< the system has completed a dynamic initialization step
    DYNAMIC_COMPLETE = 4,               //!< the system has completed a dynamic system calculation and advanced to the desired time successfully
    DYNAMIC_PARTIAL = 5,             //!< the system has completed a dynamic system calculation but has not reached the desired time
  };
  int consolePrintLevel = GD_SUMMARY_PRINT;       //!< logging level for printing to the console
  int logPrintLevel = GD_SUMMARY_PRINT;           //!< logging level for saving to a file (if a file was specified)
protected:
  std::string logFile;                                          //!< log file name

  std::string stateFile;                                        //!<record file for the state
  std::string recordDirectory;                                  //!<folder location for storing recorded files
  double state_record_period = -1.0;                            //!<how often to record the state


  std::ofstream logFileStream;  //!< logging file stream


  std::shared_ptr<functionEventAdapter> stateRecorder;          //!<a recorder for recording the state on a periodic basis
  gridState_t pState = gridState_t::STARTUP;                    //!< the system state keeps track of which state the solver is in
  int errorCode = GS_NO_ERROR;                                  //!< for storage of an ERROR code if one exists (intended to be expandable to children objects so using int instead of enum)
  count_t alertCount = 0;                                       //!< count the number of alerts
  count_t eventCount = 0;                                       //!< count the number of events that are added
  count_t relayCount = 0;                                       //!< count of the number of relays added
  count_t warnCount = 0;                                        //!<  count the number of warnings from objects
  count_t errorCount = 0;                                       //!<count the number of logged warnings
  // ---------------- clock ----------------

  double startTime = 0.0;                                           //!< [s]  start time
  double stopTime = 0.0;                                            //!< [s]  end time
  double timeCurr = kNullVal;                                       //!< [s]  current time
  double stepTime = 0.05;                                           //!< [s]  time step
  double timeReturn = 0.0;                                          //!< [s]  time returned by the solver
  double nextStopTime = -kBigNum;                                   //!< next time to stop the dynamic Simulation

  double minUpdateTime = 0.0001;                                    //!<minimum time period to go between updates; for the hybrid simultaneous partitioned solution
  double maxUpdateTime = kBigNum;                                   //!<(s) max time period to go between updates
  double absTime = 0;                                               //!< [s] seconds in unix time of the system start time;


  // ---------------- recorders ----------------
  double recordStart = -kBigNum;                                         //!< [s]  recorder start time
  double recordStop = kBigNum;                                          //!< [s]  recorder stop time
  std::vector < std::shared_ptr < gridRecorder >> recordList;               //!< vector storing recorder objects
  count_t recordCount = 0;                                   //!< number of recorders
  double nextRecordTime = kBigNum;                             //!<time for the next set of recorders

  double lastStateRecordTime = -kBigNum;                      //!<last time the full state was recorded

  // ----------------timestepP -----------------
  std::shared_ptr<eventQueue> EvQ;       //!< the event queue for the simulation system
  /** @brief storage location for shared_ptrs to gridCoreObjects
   the direct pointer to the object will get passed to the system but the ownership will be changed so it won't be deleted by the normal means
  this allows storage of shared_ptrs to modeled objects but also other objects that potentially act as storage containers, do periodic updates, generate alerts or
  interact with other simulations
  */
  std::vector<std::shared_ptr<gridCoreObject> > extraObjects;
public:
  /** @brief constructor*/
  gridSimulation (const std::string &objName = "sim_#");

  /** @brief destructor
  */
  ~gridSimulation ();
  gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;

  /** @brief get the current state of the simulation
  @return the current state
  */
  gridState_t currentProcessState () const
  {
    return pState;
  }
  /** @brief set the error code,  also sets the state to GD_ERROR
  @param[in] the error code
  */
  void setErrorCode (int ecode);
  /** @brief get the current error code
  @return the current error code
  */
  int getErrorCode () const
  {
    return errorCode;
  }

  // add components
  using gridArea::add;       //use the add function of gridArea
  virtual int addsp (std::shared_ptr<gridCoreObject> obj) override;
  /** @brief function to add recorders to the system
  @param[in] rec the recorder to add into the simulation
  */
  virtual int add (std::shared_ptr<gridRecorder> rec);
  /** @brief function to add events to the system
  @param[in] evnt the event to add into the simulation
  */
  virtual int add (std::shared_ptr<gridEvent> evnt);
  /** @brief function to add a list of events to the system
  @param[in] elist a list of events to add in
  */
  virtual int add (std::list < std::shared_ptr < gridEvent >> elist);
  /** @brief function to add an event Adpater to the event Queue
  @param[in] eA the eventAdpater to add
  */
  virtual int add (std::shared_ptr<eventAdapter> eA);

  /** @brief reset all object counters to 0*/
  static void resetObjectCounters ();

  /** @brief function to find a specific recorder by name
  @param[in] recordname  the name of the recorder to find
  @return a shared_ptr to the recorder that was found or an empty shared ptr*/
  std::shared_ptr<gridRecorder> findRecorder (const std::string recordname);

/** @brief run the simulator
@param[in] finishTime  the time to run to
@return return code 0 for success other for failure
*/
  virtual int run (double finishTime = kNullVal);
  /** @brief have the simulator step forward in time
  @return return code 0 for success other for failure
  */
  virtual int step ();

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;

  virtual std::string getString (const std::string &param) const override;
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;

  void alert (gridCoreObject *object, int code) override;
  virtual void log (gridCoreObject *object,int level, const std::string &message) override;

  /** @brief save all the recorder data to files
   all the recorders have files associated with them that get automatically saved at certain points this function forces them
  to do a save operation*/
  void saveRecorders ();

  /**
   * \brief Gets the current simulation time.
   * \return a double representing the current simulation time, in seconds.
   */
  double getCurrentTime () const
  {
    return timeCurr;
  }

  /**
   * \brief Gets the simulation start time.
   * \return a double representing the simulation start time, in seconds.
   */
  double getStartTime () const
  {
    return startTime;
  }

  /**
  * \brief gets the next event time.
  * \return a double representing the next scheduled event in Griddyn.
  */
  double getEventTime () const;
};

/** @brief find an object that has the same properties as obj1 located int the tree from src in the tree given by sec
@param[in] obj1  the object to search for
@param[in] src  the tree the current object is associated with
@param[in] sec the tree to do the search in
@return the located object or nullptr
*/
gridCoreObject * findMatchingObject (gridCoreObject *obj1, gridPrimary *src, gridPrimary *sec);

#endif
