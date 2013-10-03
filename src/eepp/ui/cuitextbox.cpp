#include <eepp/ui/cuitextbox.hpp>
#include <eepp/ui/cuimanager.hpp>
#include <eepp/ui/cuithememanager.hpp>
#include <eepp/graphics/ctextcache.hpp>
#include <eepp/graphics/cfont.hpp>

namespace EE { namespace UI {

cUITextBox::cUITextBox( const cUITextBox::CreateParams& Params ) :
	cUIComplexControl( Params ),
	mFontColor( Params.FontColor ),
	mFontShadowColor( Params.FontShadowColor ),
	mAlignOffset( 0.f, 0.f )
{
	mTextCache = eeNew( cTextCache, () );
	mTextCache->Font( Params.Font );
	mTextCache->Color( mFontColor );
	mTextCache->ShadowColor( mFontShadowColor );

	if ( NULL == Params.Font ) {
		if ( NULL != cUIThemeManager::instance()->DefaultFont() )
			mTextCache->Font( cUIThemeManager::instance()->DefaultFont() );
		else
			eePRINTL( "cUITextBox::cUITextBox : Created a UI TextBox without a defined font." );
	}

	AutoAlign();
}

cUITextBox::~cUITextBox() {
	eeSAFE_DELETE( mTextCache );
}

Uint32 cUITextBox::Type() const {
	return UI_TYPE_TEXTBOX;
}

bool cUITextBox::IsType( const Uint32& type ) const {
	return cUITextBox::Type() == type ? true : cUIComplexControl::IsType( type );
}

void cUITextBox::Draw() {
	if ( mVisible && 0.f != mAlpha ) {
		cUIControlAnim::Draw();

		if ( mTextCache->GetTextWidth() ) {
			if ( mFlags & UI_CLIP_ENABLE ) {
				cUIManager::instance()->ClipEnable(
						mScreenPos.x + mPadding.Left,
						mScreenPos.y + mPadding.Top,
						mSize.Width() - mPadding.Left - mPadding.Right,
						mSize.Height() - mPadding.Top - mPadding.Bottom
				);
			}

			mTextCache->Flags( Flags() );
			mTextCache->Draw( (eeFloat)mScreenPos.x + mAlignOffset.x + (eeFloat)mPadding.Left, (eeFloat)mScreenPos.y + mAlignOffset.y + (eeFloat)mPadding.Top, 1.f, 0.f, Blend() );

			if ( mFlags & UI_CLIP_ENABLE ) {
				cUIManager::instance()->ClipDisable();
			}
		}
	}
}

cFont * cUITextBox::Font() const {
	return mTextCache->Font();
}

void cUITextBox::Font( cFont * font ) {
	if ( mTextCache->Font() != font ) {
		mTextCache->Font( font );
		AutoShrink();
		AutoSize();
		AutoAlign();
		OnFontChanged();
	}
}

const String& cUITextBox::Text() {
	if ( mFlags & UI_AUTO_SHRINK_TEXT )
		return mString;

	return mTextCache->Text();
}

void cUITextBox::Text( const String& text ) {
	if ( mFlags & UI_AUTO_SHRINK_TEXT ) {
		mString = text;
		mTextCache->Text( mString );
	} else {
		mTextCache->Text( text );
	}

	AutoShrink();
	AutoSize();
	AutoAlign();
	OnTextChanged();
}

const eeColorA& cUITextBox::Color() const {
	return mFontColor;
}

void cUITextBox::Color( const eeColorA& color ) {
	mFontColor = color;
	mTextCache->Color( color );

	Alpha( color.A() );
}

const eeColorA& cUITextBox::ShadowColor() const {
	return mFontShadowColor;
}

void cUITextBox::ShadowColor( const eeColorA& color ) {
	mFontShadowColor = color;
	mTextCache->ShadowColor( mFontColor );
}

void cUITextBox::Alpha( const eeFloat& alpha ) {
	cUIControlAnim::Alpha( alpha );
	mFontColor.Alpha = (Uint8)alpha;
	mFontShadowColor.Alpha = (Uint8)alpha;

	mTextCache->Alpha( mFontColor.Alpha );
}

void cUITextBox::AutoShrink() {
	if ( mFlags & UI_AUTO_SHRINK_TEXT ) {
		ShrinkText( mSize.Width() );
	}
}

void cUITextBox::ShrinkText( const Uint32& MaxWidth ) {
	if ( mFlags & UI_AUTO_SHRINK_TEXT ) {
		mTextCache->Text( mString );
	}

	mTextCache->Font()->ShrinkText( mTextCache->Text(), MaxWidth );
}

void cUITextBox::AutoSize() {
	if ( mFlags & UI_AUTO_SIZE ) {
		mSize.Width( (eeInt)mTextCache->GetTextWidth() );
		mSize.Height( (eeInt)mTextCache->GetTextHeight() );
	}
}

void cUITextBox::AutoAlign() {
	switch ( FontHAlignGet( Flags() ) ) {
		case UI_HALIGN_CENTER:
			mAlignOffset.x = (eeFloat)( (Int32)( mSize.x - mTextCache->GetTextWidth() ) / 2 );
			break;
		case UI_HALIGN_RIGHT:
			mAlignOffset.x = ( (eeFloat)mSize.x - (eeFloat)mTextCache->GetTextWidth() );
			break;
		case UI_HALIGN_LEFT:
			mAlignOffset.x = 0.f;
			break;
	}

	switch ( FontVAlignGet( Flags() ) ) {
		case UI_VALIGN_CENTER:
			mAlignOffset.y = (eeFloat)( ( (Int32)( mSize.y - mTextCache->GetTextHeight() ) ) / 2 ) - 1;
			break;
		case UI_VALIGN_BOTTOM:
			mAlignOffset.y = ( (eeFloat)mSize.y - (eeFloat)mTextCache->GetTextHeight() );
			break;
		case UI_VALIGN_TOP:
			mAlignOffset.y = 0.f;
			break;
	}
}

void cUITextBox::OnSizeChange() {
	AutoShrink();
	AutoSize();
	AutoAlign();

	cUIControlAnim::OnSizeChange();

	mTextCache->Cache();
}

void cUITextBox::OnTextChanged() {
	SendCommonEvent( cUIEvent::EventOnTextChanged );
}

void cUITextBox::OnFontChanged() {
	SendCommonEvent( cUIEvent::EventOnFontChanged );
}

void cUITextBox::Padding( const eeRecti& padding ) {
	mPadding = padding;
}

const eeRecti& cUITextBox::Padding() const {
	return mPadding;
}

void cUITextBox::SetTheme( cUITheme * Theme ) {
	cUIControlAnim::SetTheme( Theme );

	if ( NULL == mTextCache->Font() && NULL != Theme->Font() ) {
		mTextCache->Font( Theme->Font() );
	}
}

cTextCache * cUITextBox::GetTextCache() {
	return mTextCache;
}

eeFloat cUITextBox::GetTextWidth() {
	return mTextCache->GetTextWidth();
}

eeFloat cUITextBox::GetTextHeight() {
	return mTextCache->GetTextHeight();
}

const eeInt& cUITextBox::GetNumLines() const {
	return mTextCache->GetNumLines();
}

const eeVector2f& cUITextBox::AlignOffset() const {
	return mAlignOffset;
}

}}
