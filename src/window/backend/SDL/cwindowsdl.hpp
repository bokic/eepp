#ifndef EE_WINDOWCWINDOWSDL_HPP
#define EE_WINDOWCWINDOWSDL_HPP

#include "../../cbackend.hpp"
#include "base.hpp"

#ifdef EE_BACKEND_SDL_1_2

#include "../../cwindow.hpp"

#if EE_PLATFORM == EE_PLATFORM_WIN || EE_PLATFORM == EE_PLATFORM_MACOSX || defined( EE_X11_PLATFORM )
#include <SDL/SDL_syswm.h>
#endif

namespace EE { namespace Window { namespace Backend { namespace SDL {

class EE_API cWindowSDL : public cWindow {
	public:
		cWindowSDL( WindowSettings Settings, ContextSettings Context );

		virtual ~cWindowSDL();

		bool Create( WindowSettings Settings, ContextSettings Context );

		void ToggleFullscreen();

		void Caption( const std::string& Caption );

		bool Icon( const std::string& Path );

		bool Active();

		bool Visible();

		void Size( Uint32 Width, Uint32 Height, bool Windowed );

		std::vector< std::pair<unsigned int, unsigned int> > GetPossibleResolutions() const;

		void SetGamma( eeFloat Red, eeFloat Green, eeFloat Blue );

		eeWindowHandler	GetWindowHandler();
	protected:
		friend class cClipboardSDL;

		SDL_Surface *	mSurface;

		#if EE_PLATFORM == EE_PLATFORM_WIN || EE_PLATFORM == EE_PLATFORM_MACOSX || defined( EE_X11_PLATFORM )
		SDL_SysWMinfo 	mWMinfo;
		#endif

		eeVector2i		mWinPos;

		void CreatePlatform();

		void SwapBuffers();

		void SetGLConfig();

		std::string GetVersion();
};

}}}}

#endif

#endif
