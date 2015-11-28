#include <eepp/window/clipboard.hpp>

namespace EE { namespace Window {

Clipboard::Clipboard( EE::Window::Window * window ) :
	mWindow( window )
{}

Clipboard::~Clipboard() {}

EE::Window::Window * Clipboard::GetWindow() const {
	return mWindow;
}

}}
