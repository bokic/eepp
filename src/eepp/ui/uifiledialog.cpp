#include <algorithm>
#include <eepp/system/filesystem.hpp>
#include <eepp/ui/uifiledialog.hpp>
#include <eepp/ui/uilinearlayout.hpp>
#include <eepp/ui/uilistboxitem.hpp>
#include <eepp/ui/uiscenenode.hpp>
#include <eepp/ui/uistyle.hpp>
#include <eepp/ui/uithememanager.hpp>

namespace EE { namespace UI {

#define FDLG_MIN_WIDTH 420
#define FDLG_MIN_HEIGHT 320

UIFileDialog* UIFileDialog::New( Uint32 dialogFlags, std::string defaultFilePattern,
								 std::string defaultDirectory ) {
	return eeNew( UIFileDialog, ( dialogFlags, defaultFilePattern, defaultDirectory ) );
}

UIFileDialog::UIFileDialog( Uint32 dialogFlags, std::string defaultFilePattern,
							std::string defaultDirectory ) :
	UIWindow(),
	mCurPath( defaultDirectory ),
	mDialogFlags( dialogFlags ),
	mCloseWithKey( KEY_UNKNOWN ) {
	if ( getSize().getWidth() < FDLG_MIN_WIDTH ) {
		mDpSize.x = FDLG_MIN_WIDTH;
		mSize.x = PixelDensity::dpToPxI( FDLG_MIN_WIDTH );
	}

	if ( getSize().getHeight() < FDLG_MIN_HEIGHT ) {
		mDpSize.y = FDLG_MIN_HEIGHT;
		mSize.y = PixelDensity::dpToPxI( FDLG_MIN_HEIGHT );
	}

	if ( mStyleConfig.MinWindowSize.getWidth() < FDLG_MIN_WIDTH )
		mStyleConfig.MinWindowSize.setWidth( FDLG_MIN_WIDTH );

	if ( mStyleConfig.MinWindowSize.getHeight() < FDLG_MIN_HEIGHT )
		mStyleConfig.MinWindowSize.setHeight( FDLG_MIN_HEIGHT );

	bool loading = isSceneNodeLoading();
	mUISceneNode->setIsLoading( true );

	mContainer->setSize( getSize() );

	if ( getAllowFolderSelect() ) {
		setTitle( "Select a folder" );
	} else {
		setTitle( "Select a file" );
	}

	UILinearLayout* linearLayout = UILinearLayout::NewVertical();
	linearLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::MatchParent )
		->setLayoutMargin( Rect( 4, 2, 4, 2 ) )
		->setParent( getContainer() );

	UILinearLayout* hLayout = UILinearLayout::NewHorizontal();
	hLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setLayoutMargin( Rect( 0, 0, 0, 4 ) )
		->setParent( linearLayout )
		->setId( "lay1" );

	UITextView::New()
		->setText( "Look in:" )
		->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::MatchParent )
		->setLayoutMargin( Rect( 0, 0, 4, 0 ) )
		->setParent( hLayout )
		->setEnabled( false );

	mPath = UITextInput::New();
	mPath->setText( mCurPath )
		->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::WrapContent )
		->setLayoutWeight( 1 )
		->setParent( hLayout );
	mPath->addEventListener( Event::OnPressEnter, cb::Make1( this, &UIFileDialog::onPressEnter ) );

	mButtonUp = UIPushButton::New();
	mButtonUp->setText( "Up" )
		->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::MatchParent )
		->setParent( hLayout );

	mList = UIListBox::New();
	mList->setParent( linearLayout );
	mList->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setLayoutWeight( 1 )
		->setLayoutMargin( Rect( 0, 0, 0, 4 ) );
	mList->addEventListener( Event::KeyDown, [&]( const Event* event ) {
		const KeyEvent* KEvent = reinterpret_cast<const KeyEvent*>( event );
		if ( KEvent->getKeyCode() == KEY_BACKSPACE ) {
			goFolderUp();
		}
	} );
	mList->addEventListener( Event::OnItemKeyDown, [&]( const Event* event ) {
		const KeyEvent* KEvent = reinterpret_cast<const KeyEvent*>( event );
		if ( KEvent->getKeyCode() == KEY_RETURN ) {
			openFileOrFolder();
		} else if ( KEvent->getKeyCode() == KEY_BACKSPACE ) {
			goFolderUp();
		}
	} );

	hLayout = UILinearLayout::NewHorizontal();
	hLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setLayoutMargin( Rect( 0, 0, 0, 4 ) )
		->setParent( linearLayout );

	UITextView::New()
		->setText( "File Name:" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::MatchParent )
		->setSize( 74, 0 )
		->setParent( hLayout )
		->setEnabled( false );

	mFile = UITextInput::New();
	mFile->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::MatchParent )
		->setLayoutWeight( 1 )
		->setParent( hLayout );
	mFile->setLayoutMargin( Rect( 0, 0, 4, 0 ) );
	mFile->addEventListener( Event::OnPressEnter,
							 cb::Make1( this, &UIFileDialog::onPressFileEnter ) );

	mButtonOpen = UIPushButton::New();
	mButtonOpen->setText( isSaveDialog() ? "Save" : "Open" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::WrapContent )
		->setSize( 80, 0 )
		->setParent( hLayout );

	hLayout = UILinearLayout::NewHorizontal();
	hLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setParent( linearLayout );

	UITextView::New()
		->setText( "Files of type:" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::MatchParent )
		->setSize( 74, 0 )
		->setParent( hLayout )
		->setEnabled( false );

	mFiletype = UIDropDownList::New();
	mFiletype->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::WrapContent )
		->setLayoutWeight( 1 )
		->setParent( hLayout );
	mFiletype->setPopUpToRoot( true );
	mFiletype->getListBox()->addListBoxItem( defaultFilePattern );
	mFiletype->getListBox()->setSelected( 0 );
	mFiletype->setLayoutMargin( Rect( 0, 0, 4, 0 ) );

	mButtonCancel = UIPushButton::New();
	mButtonCancel->setText( "Cancel" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::WrapContent )
		->setSize( 80, 0 )
		->setParent( hLayout );

	applyDefaultTheme();

	mUISceneNode->setIsLoading( loading );
}

UIFileDialog::~UIFileDialog() {}

void UIFileDialog::onWindowReady() {
	updateClickStep();
}

Uint32 UIFileDialog::getType() const {
	return UI_TYPE_FILEDIALOG;
}

bool UIFileDialog::isType( const Uint32& type ) const {
	return UIFileDialog::getType() == type ? true : UIWindow::isType( type );
}

void UIFileDialog::setTheme( UITheme* Theme ) {
	UIWindow::setTheme( Theme );

	mButtonOpen->setTheme( Theme );
	mButtonCancel->setTheme( Theme );
	mButtonUp->setTheme( Theme );
	mList->setTheme( Theme );
	mPath->setTheme( Theme );
	mFile->setTheme( Theme );
	mFiletype->setTheme( Theme );

	Drawable* Icon = Theme->getIconByName( "go-up" );

	if ( NULL != Icon ) {
		mButtonUp->setText( "" );
		mButtonUp->setIcon( Icon );
	}

	onThemeLoaded();
}

void UIFileDialog::refreshFolder() {
	std::vector<String> flist = FileSystem::filesGetInPath( String( mCurPath ) );
	std::vector<String> files;
	std::vector<String> folders;
	std::vector<std::string> patterns;
	bool accepted;
	Uint32 i, z;

	if ( "*" != mFiletype->getText() ) {
		patterns = String::split( mFiletype->getText().toUtf8(), ';' );

		for ( i = 0; i < patterns.size(); i++ )
			patterns[i] = FileSystem::fileExtension( patterns[i] );
	}

	for ( i = 0; i < flist.size(); i++ ) {
		if ( getFoldersFirst() && FileSystem::isDirectory( mCurPath + flist[i] ) ) {
			folders.push_back( flist[i] );
		} else {
			accepted = false;

			if ( patterns.size() ) {
				for ( z = 0; z < patterns.size(); z++ ) {
					if ( patterns[z] == FileSystem::fileExtension( flist[i] ) ) {
						accepted = true;
						break;
					}
				}
			} else {
				accepted = true;
			}

			if ( accepted )
				files.push_back( flist[i] );
		}
	}

	if ( getSortAlphabetically() ) {
		std::sort( folders.begin(), folders.end() );
		std::sort( files.begin(), files.end() );
	}

	mList->clear();

	if ( getFoldersFirst() ) {
		mList->addListBoxItems( folders );
	}

	mList->addListBoxItems( files );

	updateClickStep();

	mList->setFocus();
}

void UIFileDialog::updateClickStep() {
	if ( NULL != mList->getVerticalScrollBar() ) {
		mList->getVerticalScrollBar()->setClickStep(
			1.f / ( ( mList->getCount() * mList->getRowHeight() ) /
					(Float)mList->getSize().getHeight() ) );
	}
}

void UIFileDialog::openSaveClick() {
	if ( isSaveDialog() ) {
		save();
	} else {
		open();
	}
}

void UIFileDialog::onPressFileEnter( const Event* ) {
	openSaveClick();
}

void UIFileDialog::disableButtons() {
	mButtonOpen->setEnabled( false );
	mButtonCancel->setEnabled( false );
	mButtonUp->setEnabled( false );

	if ( NULL != mButtonClose )
		mButtonClose->setEnabled( false );

	if ( NULL != mButtonMinimize )
		mButtonMinimize->setEnabled( false );

	if ( NULL != mButtonMaximize )
		mButtonMaximize->setEnabled( false );
}

void UIFileDialog::openFileOrFolder() {
	std::string newPath = mCurPath + mList->getItemSelectedText();

	if ( FileSystem::isDirectory( newPath ) ) {
		mCurPath = newPath + FileSystem::getOSSlash();
		mPath->setText( mCurPath );
		refreshFolder();
	} else {
		open();
	}
}

void UIFileDialog::goFolderUp() {
	mCurPath = FileSystem::removeLastFolderFromPath( mCurPath );
	mPath->setText( mCurPath );
	refreshFolder();
}

Uint32 UIFileDialog::onMessage( const NodeMessage* Msg ) {
	switch ( Msg->getMsg() ) {
		case NodeMessage::Click: {
			if ( Msg->getFlags() & EE_BUTTON_LMASK ) {
				if ( Msg->getSender() == mButtonOpen ) {
					openSaveClick();
				} else if ( Msg->getSender() == mButtonCancel ) {
					disableButtons();

					closeWindow();
				} else if ( Msg->getSender() == mButtonUp ) {
					goFolderUp();
				}
			}

			break;
		}
		case NodeMessage::DoubleClick: {
			if ( Msg->getFlags() & EE_BUTTON_LMASK ) {
				if ( Msg->getSender()->isType( UI_TYPE_LISTBOXITEM ) ) {
					openFileOrFolder();
				}
			}

			break;
		}
		case NodeMessage::Selected: {
			if ( Msg->getSender() == mList ) {
				if ( !isSaveDialog() ) {
					if ( getAllowFolderSelect() ) {
						mFile->setText( mList->getItemSelectedText() );
					} else {
						if ( !FileSystem::isDirectory( getTempFullPath() ) ) {
							mFile->setText( mList->getItemSelectedText() );
						}
					}
				} else {
					if ( !FileSystem::isDirectory( getTempFullPath() ) ) {
						mFile->setText( mList->getItemSelectedText() );
					}
				}
			} else if ( Msg->getSender() == mFiletype ) {
				refreshFolder();
			}

			break;
		}
	}

	return UIWindow::onMessage( Msg );
}

void UIFileDialog::save() {
	sendCommonEvent( Event::SaveFile );

	disableButtons();

	closeWindow();
}

void UIFileDialog::open() {
	if ( "" != mList->getItemSelectedText() || getAllowFolderSelect() ) {
		if ( !getAllowFolderSelect() ) {
			if ( FileSystem::isDirectory( getFullPath() ) )
				return;
		} else {
			if ( !FileSystem::isDirectory( getFullPath() ) &&
				 !FileSystem::isDirectory( getCurPath() ) )
				return;
		}

		sendCommonEvent( Event::OpenFile );

		disableButtons();

		closeWindow();
	}
}

void UIFileDialog::onPressEnter( const Event* ) {
	if ( FileSystem::isDirectory( mPath->getText() ) ) {
		std::string tpath = mPath->getText();
		FileSystem::dirPathAddSlashAtEnd( tpath );
		mPath->setText( tpath );
		mCurPath = mPath->getText();
		refreshFolder();
	}
}

void UIFileDialog::addFilePattern( std::string pattern, bool select ) {
	Uint32 index = mFiletype->getListBox()->addListBoxItem( pattern );

	if ( select ) {
		mFiletype->getListBox()->setSelected( index );

		refreshFolder();
	}
}

bool UIFileDialog::isSaveDialog() {
	return 0 != ( mDialogFlags & SaveDialog );
}

bool UIFileDialog::getSortAlphabetically() {
	return 0 != ( mDialogFlags & SortAlphabetically );
}

bool UIFileDialog::getFoldersFirst() {
	return 0 != ( mDialogFlags & FoldersFirst );
}

bool UIFileDialog::getAllowFolderSelect() {
	return 0 != ( mDialogFlags & AllowFolderSelect );
}

void UIFileDialog::setSortAlphabetically( const bool& sortAlphabetically ) {
	BitOp::setBitFlagValue( &mDialogFlags, SortAlphabetically, sortAlphabetically ? 1 : 0 );
	refreshFolder();
}

void UIFileDialog::setFoldersFirst( const bool& foldersFirst ) {
	BitOp::setBitFlagValue( &mDialogFlags, FoldersFirst, foldersFirst ? 1 : 0 );
	refreshFolder();
}

void UIFileDialog::setAllowFolderSelect( const bool& allowFolderSelect ) {
	BitOp::setBitFlagValue( &mDialogFlags, AllowFolderSelect, allowFolderSelect ? 1 : 0 );
}

std::string UIFileDialog::getFullPath() {
	std::string tPath = mCurPath;

	FileSystem::dirPathAddSlashAtEnd( tPath );

	tPath += getCurFile();

	return tPath;
}

std::string UIFileDialog::getTempFullPath() {
	std::string tPath = mCurPath;

	FileSystem::dirPathAddSlashAtEnd( tPath );

	tPath += mList->getItemSelectedText().toUtf8();

	return tPath;
}

std::string UIFileDialog::getCurPath() const {
	return mCurPath;
}

std::string UIFileDialog::getCurFile() const {
	if ( mDialogFlags & SaveDialog )
		return mFile->getText();

	return mList->getItemSelectedText().toUtf8();
}

UIPushButton* UIFileDialog::getButtonOpen() const {
	return mButtonOpen;
}

UIPushButton* UIFileDialog::getButtonCancel() const {
	return mButtonCancel;
}

UIPushButton* UIFileDialog::getButtonUp() const {
	return mButtonUp;
}

UIListBox* UIFileDialog::getList() const {
	return mList;
}

UITextInput* UIFileDialog::getPathInput() const {
	return mPath;
}

UITextInput* UIFileDialog::getFileInput() const {
	return mFile;
}

UIDropDownList* UIFileDialog::getFiletypeList() const {
	return mFiletype;
}

Uint32 UIFileDialog::onKeyUp( const KeyEvent& Event ) {
	if ( mCloseWithKey && Event.getKeyCode() == mCloseWithKey ) {
		disableButtons();

		closeWindow();
	}

	return 1;
}

const Uint32& UIFileDialog::getCloseWithKey() const {
	return mCloseWithKey;
}

void UIFileDialog::setCloseWithKey( const Uint32& closeWithKey ) {
	mCloseWithKey = closeWithKey;
}

}} // namespace EE::UI