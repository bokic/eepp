#include <eepp/ui/cuiskinsimple.hpp>
#include <eepp/graphics/textureatlasmanager.hpp>

namespace EE { namespace UI {

cUISkinSimple::cUISkinSimple( const std::string& Name ) :
	cUISkin( Name, UISkinSimple )
{
	for ( Int32 i = 0; i < cUISkinState::StateCount; i++ )
		mSubTexture[ i ] = NULL;

	SetSkins();
}

cUISkinSimple::~cUISkinSimple() {
}

void cUISkinSimple::Draw( const Float& X, const Float& Y, const Float& Width, const Float& Height, const Uint32& Alpha, const Uint32& State ) {
	if ( 0 == Alpha )
		return;

	SubTexture * tSubTexture = mSubTexture[ State ];
	mTempColor		= mColor[ State ];

	if ( NULL != tSubTexture ) {
		tSubTexture->DestSize( Sizef( Width, Height ) );

		if ( mTempColor.Alpha != Alpha ) {
			mTempColor.Alpha = (Uint8)( (Float)mTempColor.Alpha * ( (Float)Alpha / 255.f ) );
		}

		tSubTexture->Draw( X, Y, mTempColor );

		tSubTexture->ResetDestSize();
	}
}

void cUISkinSimple::SetSkin( const Uint32& State ) {
	eeASSERT ( State < cUISkinState::StateCount );

	std::string Name( mName + "_" + cUISkin::GetSkinStateName( State ) );

	mSubTexture[ State ] = TextureAtlasManager::instance()->GetSubTextureByName( Name );
}

SubTexture * cUISkinSimple::GetSubTexture( const Uint32& State ) const {
	eeASSERT ( State < cUISkinState::StateCount );

	return mSubTexture[ State ];
}

void cUISkinSimple::StateNormalToState( const Uint32& State ) {
	if ( NULL == mSubTexture[ State ] )
		mSubTexture[ State ] = mSubTexture[ cUISkinState::StateNormal ];
}

cUISkinSimple * cUISkinSimple::Copy( const std::string& NewName, const bool& CopyColorsState ) {
	cUISkinSimple * SkinS = eeNew( cUISkinSimple, ( NewName ) );

	if ( CopyColorsState ) {
		SkinS->mColorDefault = mColorDefault;

		memcpy( &SkinS->mColor[0], &mColor[0], cUISkinState::StateCount * sizeof(ColorA) );
	}

	memcpy( &SkinS->mSubTexture[0], &mSubTexture[0], cUISkinState::StateCount * sizeof(SubTexture*) );

	return SkinS;
}

cUISkin * cUISkinSimple::Copy() {
	return Copy( mName, true );
}

}}
