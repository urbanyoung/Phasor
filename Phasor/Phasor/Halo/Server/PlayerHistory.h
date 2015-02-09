/********************************************************************************
	Copyright (C) 2012 PaulusT

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************/
#pragma once

#include <map>
#include "../Game/Objects.h"
//#include <Windows.h>

//extern LARGE_INTEGER countsPerSecond;

class OldData {
public:
  int valid;
  vect3d pos;
};

template<size_t MAX_ENTRIES>
class PlayerHistory
{
private:
  OldData previousPositions[MAX_ENTRIES];
  size_t nextWriteIndex;
  
  bool cleared;
public:
  PlayerHistory()
  {
    cleared = false;
    reset();
  }
  
  void reset()
  {
    if(!cleared)
    {
      nextWriteIndex = 0;
      
      memset(previousPositions, 0, sizeof(previousPositions));
      
      cleared=true;
    }
  }

  

  void addData(halo::objects::s_halo_object* player) {
    cleared = false;
    OldData data;
    data.pos = player->location;
    data.valid = 1;
    previousPositions[nextWriteIndex] = data;
    nextWriteIndex = wrap(nextWriteIndex+1);
  }

  void readDataIntoPlayer(int millisecondsInPast, halo::objects::s_halo_object* player) {
    size_t ticksInPast = size_t(double(millisecondsInPast)/1000.0 * 30.0);
    if(ticksInPast == 0) return;
    if(ticksInPast > MAX_ENTRIES-1) ticksInPast = MAX_ENTRIES-1;
    size_t index = wrap(nextWriteIndex - 1 - ticksInPast);
    const OldData& oldData = previousPositions[index];
    if(!oldData.valid) return;
    player->location = oldData.pos;

  }

private:
  inline size_t wrap(size_t index) {
    if(index >= 0) 
      return index % MAX_ENTRIES;
    
    while(index < 0) 
      index += MAX_ENTRIES;
    return index;
  }


};

