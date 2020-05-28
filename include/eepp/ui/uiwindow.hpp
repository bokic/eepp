#ifndef EE_UICUIWINDOW_HPP
#define EE_UICUIWINDOW_HPP

#include <eepp/ui/keyboardshortcut.hpp>
#include <eepp/ui/uipushbutton.hpp>
#include <eepp/ui/uitextview.hpp>
#include <eepp/ui/uiwidget.hpp>

namespace EE { namespace Graphics {
class FrameBuffer;
}} // namespace EE::Graphics

namespace EE { namespace UI {

class UISceneNode;

enum UIWindowFlags {
	UI_WIN_NO_DECORATION = ( 1 << 0 ),
	UI_WIN_CLOSE_BUTTON = ( 1 << 1 ),
	UI_WIN_MINIMIZE_BUTTON = ( 1 << 2 ),
	UI_WIN_MAXIMIZE_BUTTON = ( 1 << 3 ),
	UI_WIN_USE_DEFAULT_BUTTONS_ACTIONS = ( 1 << 4 ),
	UI_WIN_RESIZEABLE = ( 1 << 5 ),
	UI_WIN_DRAGABLE_CONTAINER = ( 1 << 6 ),
	UI_WIN_SHARE_ALPHA_WITH_CHILDS = ( 1 << 7 ),
	UI_WIN_MODAL = ( 1 << 8 ),
	UI_WIN_SHADOW = ( 1 << 9 ),
	UI_WIN_FRAME_BUFFER = ( 1 << 10 ),
	UI_WIN_COLOR_BUFFER = ( 1 << 11 )
};

static const Uint32 UI_WIN_DEFAULT_FLAGS = UI_WIN_CLOSE_BUTTON |
										   UI_WIN_USE_DEFAULT_BUTTONS_ACTIONS | UI_WIN_RESIZEABLE |
										   UI_WIN_SHARE_ALPHA_WITH_CHILDS;

class EE_API UIWindow : public UIWidget {
  public:
	class StyleConfig {
	  public:
		StyleConfig() {}

		StyleConfig( Uint32 winFlags ) : WinFlags( winFlags ) {}

		Uint32 WinFlags = UI_WIN_DEFAULT_FLAGS;
		Sizei TitlebarSize;
		Sizei BorderSize;
		Sizef MinWindowSize;
		Vector2i ButtonsOffset;
		Uint32 ButtonsSeparation = 4;
		Int32 MinCornerDistance = 24;
		Uint8 BaseAlpha = 255;
		bool TitlebarAutoSize = true;
		bool BorderAutoSize = true;
	};

	enum WindowBaseContainerType { SIMPLE_LAYOUT, LINEAR_LAYOUT, RELATIVE_LAYOUT };

	static UIWindow* NewOpt( WindowBaseContainerType type, const StyleConfig& windowStyleConfig );

	static UIWindow* New();

	explicit UIWindow( WindowBaseContainerType type, const StyleConfig& windowStyleConfig );

	explicit UIWindow( WindowBaseContainerType type = SIMPLE_LAYOUT );

	virtual ~UIWindow();

	virtual Uint32 getType() const;

	virtual bool isType( const Uint32& type ) const;

	virtual UINode* setSize( const Sizef& size );

	UINode* setSize( const Float& Width, const Float& Height );

	UIWindow* setSizeWithDecoration( const Float& Width, const Float& Height );

	UIWindow* setSizeWithDecoration( const Sizef& size );

	const Sizef& getSize() const;

	virtual void setTheme( UITheme* Theme );

	virtual Uint32 onMessage( const NodeMessage* Msg );

	UIWidget* getContainer() const;

	UINode* getButtonClose() const;

	UINode* getButtonMaximize() const;

	UINode* getButtonMinimize() const;

	virtual bool show();

	virtual bool hide();

	virtual void scheduledUpdate( const Time& time );

	virtual void closeWindow();

	virtual void close();

	void setWindowOpacity( const Uint8& alpha );

	const Uint8& getBaseAlpha() const;

	void setTitle( const String& Text );

	String getTitle() const;

	UITextView* getTitleTextBox() const;

	bool addShortcut( const Uint32& KeyCode, const Uint32& Mod, UIWidget* Widget );

	bool removeShortcut( const Uint32& KeyCode, const Uint32& Mod );

	bool isModal();

	UIWidget* getModalControl() const;

	void maximize();

	bool isMaximizable();

	bool isResizeable();

	Uint32 getWinFlags() const;

	UIWindow* setWinFlags( const Uint32& winFlags );

	const StyleConfig& getStyleConfig() const;

	UIWindow* setStyleConfig( const StyleConfig& styleConfig );

	UIWindow* setMinWindowSize( Sizef size );

	UIWindow* setMinWindowSize( const Float& width, const Float& height );

	const Sizef& getMinWindowSize();

	bool ownsFrameBuffer();

	virtual void loadFromXmlNode( const pugi::xml_node& node );

	virtual bool applyProperty( const StyleSheetProperty& attribute );

	virtual void internalDraw();

	void invalidate( Node* invalidator );

	bool invalidated();

	FrameBuffer* getFrameBuffer() const;

	virtual bool isDrawInvalidator() const;

	virtual std::string getPropertyString( const PropertyDefinition* propertyDef,
										   const Uint32& propertyIndex = 0 );

	std::string getWindowFlagsString();

	Sizef getMinWindowSizeWithDecoration();

	Sizef getSizeWithoutDecoration();

  protected:
	enum UI_RESIZE_TYPE {
		RESIZE_NONE,
		RESIZE_LEFT,
		RESIZE_RIGHT,
		RESIZE_TOP,
		RESIZE_BOTTOM,
		RESIZE_LEFTBOTTOM,
		RESIZE_RIGHTBOTTOM,
		RESIZE_TOPLEFT,
		RESIZE_TOPRIGHT
	};

	FrameBuffer* mFrameBuffer;
	StyleConfig mStyleConfig;
	UIWidget* mWindowDecoration;
	UIWidget* mBorderLeft;
	UIWidget* mBorderRight;
	UIWidget* mBorderBottom;
	UIWidget* mContainer;

	UIWidget* mButtonClose;
	UIWidget* mButtonMinimize;
	UIWidget* mButtonMaximize;
	UITextView* mTitle;

	UIWidget* mModalCtrl;

	Vector2f mNonMaxPos;
	Sizef mNonMaxSize;
	UI_RESIZE_TYPE mResizeType;
	Vector2f mResizePos;
	UIKeyboardShortcuts mKbShortcuts;

	bool mFrameBufferBound;

	virtual void onSizeChange();

	virtual void onAlphaChange();

	virtual void onChildCountChange( Node* child, const bool& removed );

	virtual void onPositionChange();

	virtual Uint32 onKeyDown( const KeyEvent& Event );

	virtual void matrixSet();

	virtual void matrixUnset();

	void fixChildsSize();

	void doResize( const NodeMessage* Msg );

	void decideResizeType( Node* Control );

	void tryResize( const UI_RESIZE_TYPE& getType );

	void endResize();

	void updateResize();

	void internalSize( Sizef size );

	void internalSize( const Float& w, const Float& h );

	void calcMinWinSize();

	void fixTitleSize();

	Uint32 onMouseDoubleClick( const Vector2i& position, const Uint32& flags );

	void checkShortcuts( const Uint32& KeyCode, const Uint32& Mod );

	UIKeyboardShortcuts::iterator existsShortcut( const Uint32& KeyCode, const Uint32& Mod );

	void createModalControl();

	void resizeCursor();

	void applyMinWinSize();

	void updateWinFlags();

	void createFrameBuffer();

	void drawFrameBuffer();

	void drawHighlightInvalidation();

	virtual void drawShadow();

	virtual void onPaddingChange();

	virtual void preDraw();

	virtual void postDraw();

	virtual Sizei getFrameBufferSize();

	virtual void onWindowReady();

	void onContainerPositionChange( const Event* Event );

	void setupModal();
};

}} // namespace EE::UI

#endif
