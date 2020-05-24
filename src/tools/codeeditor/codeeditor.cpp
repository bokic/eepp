#include <args/args.hxx>
#include <eepp/ee.hpp>

EE::Window::Window* win = NULL;
UISceneNode* uiSceneNode = NULL;
UICodeEditor* codeEditor = NULL;
std::string curFile = "untitled";
const std::string& windowTitle = "eepp - Code Editor";
bool docDirtyState = false;
UIMessageBox* MsgBox = NULL;

bool onCloseRequestCallback( EE::Window::Window* ) {
	if ( NULL != codeEditor && codeEditor->isDirty() ) {
		MsgBox = UIMessageBox::New(
			UIMessageBox::OK_CANCEL,
			"Do you really want to close the code editor?\nAll changes will be lost." );
		MsgBox->addEventListener( Event::MsgBoxConfirmClick,
								  []( const Event* ) { win->close(); } );
		MsgBox->addEventListener( Event::OnClose, []( const Event* ) { MsgBox = NULL; } );
		MsgBox->setTitle( "Close Code Editor?" );
		MsgBox->center();
		MsgBox->show();
		return false;
	} else {
		return true;
	}
}

void setAppTitle( const std::string& title ) {
	win->setTitle( windowTitle + String( title.empty() ? "" : " - " + title ) );
}

void loadFileFromPath( const std::string& path ) {
	codeEditor->loadFromFile( path );
	curFile = FileSystem::fileNameFromPath( path );
	setAppTitle( curFile );
}

void mainLoop() {
	if ( codeEditor->isDirty() != docDirtyState ) {
		docDirtyState = codeEditor->isDirty();
		setAppTitle( docDirtyState ? curFile + "*" : curFile );
	}

	win->getInput()->update();

	if ( win->getInput()->isControlPressed() && win->getInput()->isKeyUp( KEY_S ) ) {
		codeEditor->save();
	}

	if ( win->getInput()->isKeyUp( KEY_F6 ) ) {
		uiSceneNode->setHighlightOver( !uiSceneNode->getHighlightOver() );
	}

	if ( win->getInput()->isKeyUp( KEY_F7 ) ) {
		uiSceneNode->setDrawBoxes( !uiSceneNode->getDrawBoxes() );
	}

	if ( win->getInput()->isKeyUp( KEY_F8 ) ) {
		uiSceneNode->setDrawDebugData( !uiSceneNode->getDrawDebugData() );
	}

	if ( win->getInput()->isKeyUp( KEY_ESCAPE ) && NULL == MsgBox &&
		 onCloseRequestCallback( win ) ) {
		win->close();
	}

	// Update the UI scene.
	SceneManager::instance()->update();

	// Check if the UI has been invalidated ( needs redraw ).
	if ( SceneManager::instance()->getUISceneNode()->invalidated() ) {
		win->clear();

		// Redraw the UI scene.
		SceneManager::instance()->draw();

		win->display();
	} else {
		Sys::sleep( Milliseconds( win->isVisible() ? 1 : 8 ) );
	}
}

EE_MAIN_FUNC int main( int argc, char* argv[] ) {
	args::ArgumentParser parser( "eepp Code Editor" );
	args::Positional<std::string> file( parser, "file", "The file path" );

	try {
		parser.ParseCLI( argc, argv );
	} catch ( const args::Help& ) {
		std::cout << parser;
		return EXIT_SUCCESS;
	} catch ( const args::ParseError& e ) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return EXIT_FAILURE;
	} catch ( args::ValidationError& e ) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return EXIT_FAILURE;
	}

	Display* currentDisplay = Engine::instance()->getDisplayManager()->getDisplayIndex( 0 );
	Float pixelDensity = currentDisplay->getPixelDensity();

	win = Engine::instance()->createWindow(
		WindowSettings( 1280, 720, windowTitle, WindowStyle::Default, WindowBackend::Default, 32,
						"assets/icon/ee.png", pixelDensity ),
		ContextSettings( true ) );

	if ( win->isOpen() ) {
		win->setCloseRequestCallback( cb::Make1( onCloseRequestCallback ) );

		win->getInput()->pushCallback( []( InputEvent* event ) {
			if ( NULL == codeEditor )
				return;

			if ( event->Type == InputEvent::FileDropped ) {
				loadFileFromPath( event->file.file );
			} else if ( event->Type == InputEvent::TextDropped ) {
				codeEditor->getDocument().textInput( event->textdrop.text );
			}
		} );

		PixelDensity::setPixelDensity( eemax( win->getScale(), pixelDensity ) );

		uiSceneNode = UISceneNode::New();

		uiSceneNode->getUIThemeManager()->setDefaultFont(
			FontTrueType::New( "NotoSans-Regular", "assets/fonts/NotoSans-Regular.ttf" ) );

		FontTrueType::New( "monospace", "assets/fonts/DejaVuSansMono.ttf" );

		SceneManager::instance()->add( uiSceneNode );

		StyleSheetParser cssParser;
		if ( cssParser.loadFromFile( "assets/ui/breeze.css" ) ) {
			uiSceneNode->setStyleSheet( cssParser.getStyleSheet() );
		}

		std::string layout = R"xml(
			<LinearLayout layout_width="match_parent"
						  layout_height="match_parent"
						  orientation="vertical">
				<CodeEditor id="code_edit"
					layout_width="match_parent"
					layout_height="match_parent"
					 />
			</LinearLayout>
		)xml";
		uiSceneNode->loadLayoutFromString( layout );

		uiSceneNode->bind( "code_edit", codeEditor );
		codeEditor->setFontSize( 11 );

		if ( file ) {
			loadFileFromPath( file.Get() );
		}

		win->runMainLoop( &mainLoop );
	}

	Engine::destroySingleton();
	MemoryManager::showResults();

	return EXIT_SUCCESS;
}
