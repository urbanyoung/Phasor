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

#define DEFAULT_PING (30)

template <size_t NUM_PINGS>
class PlayerInfo
{
private:
  short pings[NUM_PINGS];
  size_t nextPingIndex;
  float averagePing;
  bool cleared;
public:
  PlayerInfo(void)
  {
    cleared = false;
    reset();
  }
  
  void reset()
  {
    if(!cleared)
    {
      cleared = true;
      nextPingIndex = 0;
      averagePing = DEFAULT_PING;
      for(size_t i = 0; i < NUM_PINGS; ++i) {
        pings[i] = DEFAULT_PING;
      }
    }
  }

  void addPing(short ping) {
    cleared = false;
    pings[nextPingIndex] = ping;
    nextPingIndex = (nextPingIndex + 1) % NUM_PINGS;
    averagePing = 0;
    for(size_t i = 0; i < NUM_PINGS; ++i) {
      averagePing += pings[(nextPingIndex + i) % NUM_PINGS];
    }
    averagePing /= NUM_PINGS;
  }

  short getAveragePing() const {
    return (short)averagePing;
  }

};

