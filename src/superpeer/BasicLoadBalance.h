
/***************************************************************************************************
*
* SUBJECT:
*    A Benckmark for Massive Multiplayer Online Games
*    Game Server and Client
*
* AUTHOR:
*    Mihai Paslariu
*    Politehnica University of Bucharest, Bucharest, Romania
*    mihplaesu@yahoo.com
*
* TIME AND PLACE:
*    University of Toronto, Toronto, Canada
*    March - August 2007
*
***************************************************************************************************/

#ifndef __BASIC_LOAD_BALANCE_H
#define __BASIC_LOAD_BALANCE_H

/***************************************************************************************************
*
* BasicLoadBalance
* - base class for all load balancing algorithms
*
***************************************************************************************************/

class BasicLoadBalance : public MasterModule
{
public:
  /* constructor / destructor */
  BasicLoadBalance(MapData &map_data, int port);
  virtual ~BasicLoadBalance();

  /* utility */
  virtual int getNumberOfPlayers();
  virtual double serverMachineFactor(int k);
  virtual double getRatio(int k);
  virtual int getLightestServer();

  /* conditions */
  virtual bool isServerOverloaded(int s); /* is server s overloaded */
  virtual bool isServerSafe(int s);
  virtual bool isServerLight(int s);
  virtual bool isOverloaded(int s, int players);  /* is server s overloaded with p players */
  virtual bool isSafe(int s, int players);
  virtual bool isLight(int s, int players);

  /* load balancing action */
  virtual bool balance() { return false; };
  virtual void initiate_action();
};

#endif
