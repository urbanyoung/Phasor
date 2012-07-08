#pragma once

namespace Core
{
	// will put codecave installer in here. something like...
	// createcodecave(addr, size, func)
	// have it call a generic function so we can (easily) track
	// calls like in old phasor. Need to pass return address to the 
	// specific function and if needed, codecaves can modify it.
	// should let us add timing to codecaves etc.
	// Not sure if I should make a codecave class, doesn't really seem
	// necessary and if we do we'll need a class to handle the codecave
	// class (or a static list tracking them). 
	// Need to add lookup table for finding right functions, should be
	// easy enough though.. will do a binary search for shits and gigs.
	// On second thoughts I probably will do a codecave class w/ a static
	// member tracking them. Makes it more OO 
	// 
	// More thinking and mind changing.. Can't (neatly) do tracking with
	// member for each codecave because 
}