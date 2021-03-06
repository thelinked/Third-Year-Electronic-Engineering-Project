#ifndef STATE_H
#define STATE_H

#include "CoreEngine.h"

namespace de
{
	namespace state
	{
		enum options
		{
			NOTHING = 0,
			EXIT = 1
		};

		class State
		{
			public:
				//virtual functions that will be overloaded
				virtual bool handleEvents( const SDL_Event &_event ) = 0;
				virtual bool logic( const float &_deltaTicks, State* &_nextState, options &_options ) = 0;

				virtual void render() = 0;
				virtual ~State(){};


			protected:
				bool changeState;
				virtual void reLoadTextures() = 0;
		};

	}
}


#endif //STATE_H
