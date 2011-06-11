#include "cmap.hpp"
#include "cgameobjectvirtual.hpp"
#include "cgameobjectshape.hpp"
#include "cgameobjectshapeex.hpp"
#include "cgameobjectsprite.hpp"
#include "ctilelayer.hpp"
#include "cobjectlayer.hpp"

#include "../graphics/cprimitives.hpp"
#include "../graphics/cshapegroupmanager.hpp"
#include "../ui/cuithememanager.hpp"
using namespace EE::Graphics;

namespace EE { namespace Gaming {

cMap::cMap() :
	mWindow( cEngine::instance()->GetCurrentWindow() ),
	mLayers( NULL ),
	mFlags( 0 ),
	mMaxLayers( 0 ),
	mLayerCount( 0 ),
	mViewSize( 800, 600 )
{
	ViewSize( mViewSize );
}

cMap::~cMap() {
	DeleteLayers();
}

void cMap::Reset() {
	DeleteLayers();

	mWindow = NULL;
	mLayers = NULL;
	mFlags	= 0;
	mMaxLayers	= 0;
	mViewSize = eeSize( 800, 600 );
}

void cMap::DeleteLayers() {
	for ( Uint32 i = 0; i < mLayerCount; i++ )
		eeSAFE_DELETE( mLayers[i] );

	eeSAFE_DELETE( mLayers );

	mLayerCount = 0;
}

void cMap::Create( eeSize Size, Uint32 MaxLayers, eeSize TileSize, Uint32 Flags, eeSize viewSize, cWindow * Window ) {
	Reset();

	mWindow		= Window;

	if ( NULL == mWindow )
		mWindow	= cEngine::instance()->GetCurrentWindow();

	mFlags		= Flags;
	mMaxLayers	= MaxLayers;
	mSize		= Size;
	mTileSize	= TileSize;
	mPixelSize	= Size * TileSize;
	mLayers		= eeNewArray( cLayer*, mMaxLayers );

	for ( Uint32 i = 0; i < mMaxLayers; i++ )
		mLayers[i] = NULL;

	ViewSize( viewSize );
}

cLayer * cMap::AddLayer( Uint32 Type, Uint32 flags, std::string name ) {
	eeASSERT( NULL != mLayers );

	if ( mLayerCount >= mMaxLayers )
		return NULL;

	switch ( Type ) {
		case MAP_LAYER_TILED:
			mLayers[ mLayerCount ] = eeNew( cTileLayer, ( this, mSize, flags, name ) );
			break;
		case MAP_LAYER_OBJECT:
			mLayers[ mLayerCount ] = eeNew( cObjectLayer, ( this, flags, name ) );
			break;
		default:
			return NULL;
	}

	mLayerCount++;

	return mLayers[ mLayerCount - 1 ];
}

cLayer* cMap::GetLayer( Uint32 index ) {
	eeASSERT( index < mLayerCount );
	return mLayers[ index ];
}

cLayer* cMap::GetLayerByHash( Uint32 hash ) {
	for ( Uint32 i = 0; i < mLayerCount; i++ ) {
		if ( mLayers[i]->Id() == hash )
			return mLayers[i];
	}

	return NULL;
}

Uint32 cMap::GetLayerIndex( cLayer * Layer ) {
	if ( NULL != Layer ) {
		for ( Uint32 i = 0; i < mLayerCount; i++ ) {
			if ( mLayers[i] == Layer )
				return i;
		}
	}

	return MAP_LAYER_UNKNOWN;
}

cLayer* cMap::GetLayer( const std::string& name ) {
	return GetLayerByHash( MakeHash( name ) );
}

void cMap::Draw() {
	cGlobalBatchRenderer::instance()->Draw();

	if ( ClipedArea() ) {
		mWindow->ClipEnable( mScreenPos.x, mScreenPos.y, mViewSize.x, mViewSize.y );
	}

	mOffsetFixed = eeVector2f( (eeFloat)mScreenPos.x, (eeFloat)mScreenPos.y ) + FixOffset();

	GetMouseOverTile();

	GridDraw();

	for ( Uint32 i = 0; i < mLayerCount; i++ ) {
		mLayers[i]->Draw( mOffsetFixed );
	}

	MouseOverDraw();

	if ( ClipedArea() ) {
		mWindow->ClipDisable();
	}
}

void cMap::MouseOverDraw() {
	if ( !DrawTileOver() )
		return;

	cPrimitives P;
	P.SetColor( eeColorA( 255, 0, 0, 255 ) );

	P.DrawRectangle( mOffsetFixed.x + mMouseOverTileFinal.x * mTileSize.x, mOffsetFixed.y + mMouseOverTileFinal.y * mTileSize.y, mTileSize.x, mTileSize.y, 0.f, 1.f, EE_DRAW_LINE );
}

void cMap::GridDraw() {
	cPrimitives P;

	P.SetColor( eeColorA( 0, 0, 0, 50 ) );
	P.DrawRectangle( mScreenPos.x, mScreenPos.y, mViewSize.x, mViewSize.y, 0.f, 1.f );
	P.SetColor( eeColorA( 255, 255, 255, 255 ) );

	if ( !DrawGrid() )
		return;

	if ( 0 == mSize.x || 0 == mSize.y )
		return;

	cGlobalBatchRenderer::instance()->Draw();

	GLi->LoadIdentity();
	GLi->PushMatrix();
	GLi->Translatef( mOffsetFixed.x, mOffsetFixed.y, 0.0f );

	eeVector2i start = StartTile();
	eeVector2i end = EndTile();

	for ( Int32 x = start.x; x < end.x; x++ ) {
		for ( Int32 y = start.y; y < end.y; y++ ) {
			P.DrawRectangle( x * mTileSize.x, y * mTileSize.y, mTileSize.x, mTileSize.y, 0.f, 1.f, EE_DRAW_LINE );
		}
	}

	GLi->PopMatrix();
}

void cMap::GetMouseOverTile() {
	eeVector2i mouse = mWindow->GetInput()->GetMousePos();

	eeVector2i MapPos( mouse.x - mScreenPos.x - mOffset.x, mouse.y - mScreenPos.y - mOffset.y );

	if ( MapPos.x < 0 )
		MapPos.x = 0;

	if ( MapPos.y < 0 )
		MapPos.y = 0;

	if ( MapPos.x > mPixelSize.x )
		MapPos.x = mPixelSize.x;

	if ( MapPos.y > mPixelSize.y )
		MapPos.y = mPixelSize.y;

	mMouseOverTile.x = MapPos.x / mTileSize.Width();
	mMouseOverTile.y = MapPos.y / mTileSize.Height();

	if ( mMouseOverTile.x < 0 )
		mMouseOverTile.x = 0;

	if ( mMouseOverTile.y < 0 )
		mMouseOverTile.y = 0;

	if ( mMouseOverTile.x >= mSize.Width() )
		mMouseOverTile.x = mSize.Width() - 1;

	if ( mMouseOverTile.y >= mSize.Height() )
		mMouseOverTile.y = mSize.Height() - 1;

	// Clamped pos
	mMouseOverTileFinal = eeVector2i( mMouseOverTile.x, mMouseOverTile.y );
	mMouseMapPos		= eeVector2i( MapPos.x, MapPos.y );
}

void cMap::Update() {
	for ( Uint32 i = 0; i < mLayerCount; i++ )
		mLayers[i]->Update();
}

const eeSize& cMap::ViewSize() const {
	return mViewSize;
}

const eeVector2i& cMap::GetMouseTilePos() const {
	return mMouseOverTileFinal;
}

const eeVector2i& cMap::GetMouseMapPos() const {
	return mMouseMapPos;
}

void cMap::ViewSize( const eeSize& viewSize ) {
	mViewSize = viewSize;

	Clamp();

	CalcTilesClip();
}

const eeVector2i& cMap::Position() const {
	return mScreenPos;
}

void cMap::Position( const eeVector2i& position ) {
	mScreenPos = position;
}

const eeVector2f& cMap::Offset() const {
	return mOffset;
}

const eeVector2i& cMap::StartTile() const {
	return mStartTile;
}

const eeVector2i& cMap::EndTile() const {
	return mEndTile;
}

void cMap::Offset( const eeVector2f& offset ) {
	mOffset = offset;

	Clamp();

	CalcTilesClip();
}

void cMap::CalcTilesClip() {
	if ( mTileSize.x > 0 && mTileSize.y > 0 ) {
		eeVector2f ffoff( FixOffset() );
		eeVector2i foff( (Int32)ffoff.x, (Int32)ffoff.y );

		mStartTile.x	= -foff.x / mTileSize.x;
		mStartTile.y	= -foff.y / mTileSize.y;
		mEndTile.x		= mStartTile.x + eeRound( (eeFloat)mViewSize.x / (eeFloat)mTileSize.x ) + 1;
		mEndTile.y		= mStartTile.y + eeRound( (eeFloat)mViewSize.y / (eeFloat)mTileSize.y ) + 1;

		if ( mStartTile.x < 0 )
			mStartTile.x = 0;

		if ( mStartTile.y < 0 )
			mStartTile.y = 0;

		if ( mEndTile.x > mSize.x )
			mEndTile.x = mSize.x;

		if ( mEndTile.y > mSize.y )
			mEndTile.y = mSize.y;
	}
}

void cMap::Clamp() {
	if ( !ClampBorders() )
		return;

	if ( mOffset.x > 0 )
		mOffset.x = 0;

	if ( mOffset.y > 0 )
		mOffset.y = 0;

	eeSize totSize( mTileSize * mSize );

	if ( -mOffset.x + mViewSize.x > totSize.x )
		mOffset.x = -( totSize.x - mViewSize.x );

	if ( -mOffset.y + mViewSize.y > totSize.y )
		mOffset.y = -( totSize.y - mViewSize.y );

	if ( totSize.x < mViewSize.x )
		mOffset.x = 0;

	if ( totSize.y < mViewSize.y )
		mOffset.y = 0;
}

void cMap::DrawGrid( const bool& draw ) {
	SetFlagValue( &mFlags, MAP_FLAG_DRAW_GRID, draw ? 1 : 0 );
}

Uint32 cMap::DrawGrid() const {
	return mFlags & MAP_FLAG_DRAW_GRID;
}

Uint32 cMap::ClipedArea() const {
	return mFlags & MAP_FLAG_CLIP_AREA;
}

Uint32 cMap::ClampBorders() const {
	return mFlags & MAP_FLAG_CLAMP_BODERS;
}

Uint32 cMap::DrawTileOver() const {
	return mFlags & MAP_FLAG_DRAW_TILE_OVER;
}

void cMap::DrawTileOver( const bool& draw ) {
	SetFlagValue( &mFlags, MAP_FLAG_DRAW_TILE_OVER, draw ? 1 : 0 );
}

eeVector2f cMap::FixOffset() {
	return eeVector2f( (eeFloat)static_cast<Int32>( mOffset.x ), (eeFloat)static_cast<Int32>( mOffset.y ) );
}

void cMap::Move( const eeVector2f& offset )  {
	Move( offset.x, offset.y );
}

void cMap::Move( const eeFloat& offsetx, const eeFloat& offsety ) {
	Offset( eeVector2f( mOffset.x + offsetx, mOffset.y + offsety ) );
}

cGameObject * cMap::CreateGameObject( const Uint32& Type, const Uint32& Flags, const Uint32& DataId ) {
	switch ( Type ) {
		case GAMEOBJECT_TYPE_SHAPE:
		{
			cGameObjectShape * tShape = eeNew( cGameObjectShape, ( Flags ) );

			tShape->DataId( DataId );

			return tShape;
		}
		case GAMEOBJECT_TYPE_SHAPEEX:
		{
			cGameObjectShapeEx * tShapeEx = eeNew( cGameObjectShapeEx, ( Flags ) );

			tShapeEx->DataId( DataId );

			return tShapeEx;
		}
		case GAMEOBJECT_TYPE_SPRITE:
		{
			cGameObjectSprite * tSprite = eeNew( cGameObjectSprite, ( Flags ) );

			tSprite->DataId( DataId );

			return tSprite;
		}
		default:
		{
			if ( mCreateGOCb.IsSet() ) {
				return mCreateGOCb( Type, Flags, DataId );
			} else {
				cGameObjectVirtual * tVirtual;
				cShape * tIsShape = cShapeGroupManager::instance()->GetShapeById( DataId );

				if ( NULL != tIsShape ) {
					tVirtual = eeNew( cGameObjectVirtual, ( tIsShape, Flags, Type ) );
				} else {
					tVirtual = eeNew( cGameObjectVirtual, ( DataId, Flags, Type ) );
				}

				return tVirtual;
			}
		}
	}

	return NULL;
}

const eeSize& cMap::TotalSize() const {
	return mPixelSize;
}

const eeSize& cMap::TileSize() const {
	return mTileSize;
}

const eeSize& cMap::Size() const {
	return mSize;
}

const Uint32& cMap::LayerCount() const {
	return mLayerCount;
}

const Uint32& cMap::MaxLayers() const {
	return mMaxLayers;
}

bool cMap::MoveLayerUp( cLayer * Layer ) {
	Uint32 Lindex = GetLayerIndex( Layer );

	if ( Lindex != MAP_LAYER_UNKNOWN && mLayerCount > 1 && ( Lindex < mLayerCount - 1 ) && ( Lindex + 1 < mLayerCount ) ) {
		cLayer * tLayer = mLayers[ Lindex + 1 ];

		mLayers[ Lindex ]		= tLayer;
		mLayers[ Lindex + 1 ]	= Layer;

		return true;
	}

	return false;
}

bool cMap::MoveLayerDown( cLayer * Layer ) {
	Uint32 Lindex = GetLayerIndex( Layer );

	if ( Lindex != MAP_LAYER_UNKNOWN && mLayerCount > 1 && Lindex >= 1 ) {
		cLayer * tLayer = mLayers[ Lindex - 1 ];

		mLayers[ Lindex ]		= tLayer;
		mLayers[ Lindex - 1 ]	= Layer;

		return true;
	}

	return false;
}

bool cMap::RemoveLayer( cLayer * Layer ) {
	Uint32 Lindex = GetLayerIndex( Layer );

	if ( Lindex != MAP_LAYER_UNKNOWN ) {
		eeSAFE_DELETE( mLayers[ Lindex ] );

		cLayer * LastLayer = NULL;

		// Reorder layers, to clean empty layers in between layers.
		for ( Uint32 i = 0; i < mLayerCount; i++ ) {
			if ( i > 0 && NULL != mLayers[i] && NULL == LastLayer ) {
				mLayers[ i - 1 ]	= mLayers[ i ];
				mLayers[ i ]		= NULL;
			}

			LastLayer = mLayers[i];
		}

		mLayerCount--;

		return true;
	}

	return false;
}

void cMap::ClearProperties() {
	mProperties.clear();
}

void cMap::AddProperty( std::string Text, std::string Value ) {
	mProperties[ Text ] = Value;
}

void cMap::EditProperty( std::string Text, std::string Value ) {
	mProperties[ Text ] = Value;
}

void cMap::RemoveProperty( std::string Text ) {
	mProperties.erase( Text );
}

cMap::PropertiesMap& cMap::GetProperties() {
	return mProperties;
}

void cMap::AddVirtualObjectType( const std::string& name ) {
	mObjTypes.push_back( name );
	mObjTypes.unique();
}

void cMap::RemoveVirtualObjectType( const std::string& name ) {
	mObjTypes.remove( name );
}

void cMap::ClearVirtualObjectTypes() {
	mObjTypes.clear();
}

cMap::GOTypesList& cMap::GetVirtualObjectTypes() {
	return mObjTypes;
}

void cMap::SetCreateGameObjectCallback( const CreateGOCb& Cb ) {
	mCreateGOCb = Cb;
}

#define MAP_PROPERTY_SIZE			(64)
#define LAYER_NAME_SIZE				(64)
#define MAP_SHAPEGROUP_PATH_SIZE	(128)

typedef struct sPropertyHdrS {
	char	Name[ MAP_PROPERTY_SIZE ];
	char	Value[ MAP_PROPERTY_SIZE ];
} sPropertyHdr;

typedef struct sMapShapeGroupS {
	char	Path[ MAP_SHAPEGROUP_PATH_SIZE ];
} sMapShapeGroup;

typedef struct sVirtualObjS {
	char	Name[ MAP_PROPERTY_SIZE ];
} sVirtualObj;

typedef struct sMapHdrS {
	Uint32	Magic;
	Uint32	SizeX;
	Uint32	SizeY;
	Uint32	TileSizeX;
	Uint32	TileSizeY;
	Uint32	MaxLayers;
	Uint32	LayerCount;
	Uint32	Flags;
	Uint32	PropertyCount;
	Uint32	ShapeGroupCount;
	Uint32	VirtualObjectTypesCount;
} sMapHdr;

typedef struct sLayerHdrS {
	char	Name[ LAYER_NAME_SIZE ];
	Uint32	Type;
	Uint32	Flags;
	Int32	OffsetX;
	Int32	OffsetY;
	Uint32	PropertyCount;
	Uint32	ObjectCount;		//! Only used by the Object Layer
} sLayerHdr;

typedef struct sMapTileGOHdrS {
	Uint32	Type;
	Uint32	Id;
	Uint32	Flags;
} sMapTileGOHdr;

typedef struct sMapObjGOHdrS {
	Uint32	Type;
	Uint32	Id;
	Uint32	Flags;
	Int32	PosX;
	Int32	PosY;
} sMapObjGOHdr;

bool cMap::Load( const std::string& path ) {
	if ( FileExists( path ) ) {
		sMapHdr MapHdr;
		Uint32 i, z;

		std::fstream fs ( path.c_str() , std::ios::in | std::ios::binary );

		if ( fs.is_open() ) {
			fs.read( reinterpret_cast<char*> (&MapHdr), sizeof(sMapHdr) );

			if ( MapHdr.Magic == ( ( 'E' << 0 ) | ( 'E' << 8 ) | ( 'M' << 16 ) | ( 'P' << 24 ) ) ) {
				Create( eeSize( MapHdr.SizeX, MapHdr.SizeY ), MapHdr.MaxLayers, eeSize( MapHdr.TileSizeX, MapHdr.TileSizeY ), MapHdr.Flags );

				//! Load Properties
				if ( MapHdr.PropertyCount ) {
					sPropertyHdr tProp[ MapHdr.PropertyCount ];
					fs.read( reinterpret_cast<char*>( &tProp[0] ), sizeof(sPropertyHdr) * MapHdr.PropertyCount );

					for ( i = 0; i < MapHdr.PropertyCount; i++ ) {
						AddProperty( std::string( tProp[i].Name ), std::string( tProp[i].Value ) );
					}
				}

				//! Load Shape Groups
				if ( MapHdr.ShapeGroupCount ) {
					sMapShapeGroup tSG[ MapHdr.ShapeGroupCount ];
					fs.read( reinterpret_cast<char*>( &tSG[0] ), sizeof(sMapShapeGroup) * MapHdr.ShapeGroupCount );

					std::vector<std::string> ShapeGroups;

					for ( i = 0; i < MapHdr.ShapeGroupCount; i++ ) {
						ShapeGroups.push_back( std::string( tSG[i].Path ) );
					}

					//! Load the Texture groups if needed
				}

				//! Load Virtual Object Types
				if ( MapHdr.VirtualObjectTypesCount ) {
					sVirtualObj tVObj[ MapHdr.VirtualObjectTypesCount ];
					fs.read( reinterpret_cast<char*>( &tVObj[0] ), sizeof(sVirtualObj) * MapHdr.VirtualObjectTypesCount );

					for ( i = 0; i < MapHdr.VirtualObjectTypesCount; i++ ) {
						AddVirtualObjectType( std::string( tVObj[i].Name ) );
					}
				}

				//! Load Layers
				if ( MapHdr.LayerCount ) {
					sLayerHdr tLayersHdr[ MapHdr.LayerCount ];
					sLayerHdr * tLayerHdr;

					fs.read( reinterpret_cast<char*>( &tLayersHdr[0] ), sizeof(sLayerHdr) * MapHdr.LayerCount );

					for ( i = 0; i < MapHdr.LayerCount; i++ ) {
						tLayerHdr = &(tLayersHdr[i]);

						cLayer * tLayer = AddLayer( tLayerHdr->Type, tLayerHdr->Flags, std::string( tLayerHdr->Name ) );

						tLayer->Offset( eeVector2f( (eeFloat)tLayerHdr->OffsetX, (eeFloat)tLayerHdr->OffsetY ) );

						sPropertyHdr tProps[ tLayerHdr->PropertyCount ];
						fs.read( reinterpret_cast<char*>( &tProps[0] ), sizeof(sPropertyHdr) * tLayerHdr->PropertyCount );

						for ( z = 0; z < tLayerHdr->PropertyCount; z++ ) {
							tLayer->AddProperty( std::string( tProps[z].Name ), std::string( tProps[z].Value ) );
						}
					}

					bool ThereIsTiled = false;

					for ( i = 0; i < mLayerCount; i++ ) {
						if ( NULL != mLayers[i] && mLayers[i]->Type() == MAP_LAYER_TILED ) {
							ThereIsTiled = true;
						}
					}

					Int32 x, y;
					Uint32 tReadFlag = 0;
					cTileLayer * tTLayer;
					cGameObject * tGO;

					if ( ThereIsTiled ) {
						//! First we read the tiled layers.
						for ( y = 0; y < mSize.y; y++ ) {
							for ( x = 0; x < mSize.x; x++ ) {

								//! Read the current tile flags
								fs.read( reinterpret_cast<char*> ( &tReadFlag ), sizeof(Uint32) );

								//! Read every game object header corresponding to this tile
								for ( i = 0; i < mLayerCount; i++ ) {
									if ( tReadFlag & ( 1 << i ) ) {
										tTLayer = reinterpret_cast<cTileLayer*> ( mLayers[i] );

										sMapTileGOHdr tTGOHdr;
										fs.read( reinterpret_cast<char*> ( &tTGOHdr ), sizeof(sMapTileGOHdr) );

										tGO = CreateGameObject( tTGOHdr.Type, tTGOHdr.Flags, tTGOHdr.Id );

										tTLayer->AddGameObject( tGO, eeVector2i( x, y ) );
									}
								}
							}
						}
					}

					//! Load the game objects from the object layers
					cObjectLayer * tOLayer;

					for ( i = 0; i < mLayerCount; i++ ) {
						if ( NULL != mLayers[i] && mLayers[i]->Type() == MAP_LAYER_OBJECT ) {
							tLayerHdr	= &( tLayersHdr[i] );
							tOLayer		= reinterpret_cast<cObjectLayer*> ( mLayers[i] );

							sMapObjGOHdr tOGOsHdr[ tLayerHdr->ObjectCount ];
							sMapObjGOHdr * tOGOHdr;

							fs.read( reinterpret_cast<char*> ( &tOGOsHdr ), sizeof(sMapObjGOHdr) * tLayerHdr->ObjectCount );

							for ( z = 0; z < tLayerHdr->ObjectCount; z++ ) {
								tOGOHdr = &( tOGOsHdr[z] );

								tGO = CreateGameObject( tOGOHdr->Type, tOGOHdr->Flags, tOGOHdr->Id );

								tGO->Pos( eeVector2f( tOGOHdr->PosX, tOGOHdr->PosY ) );

								tOLayer->AddGameObject( tGO );
							}
						}
					}
				}

				return true;
			}
		}
	}

	return false;
}

void cMap::Save( const std::string& path ) {
	Uint32 i;
	sMapHdr MapHdr;
	cLayer * tLayer;

	std::vector<std::string> ShapeGroups = GetShapeGroups();

	MapHdr.Magic					= ( ( 'E' << 0 ) | ( 'E' << 8 ) | ( 'M' << 16 ) | ( 'P' << 24 ) );
	MapHdr.Flags					= mFlags;
	MapHdr.MaxLayers				= mMaxLayers;
	MapHdr.SizeX					= mSize.Width();
	MapHdr.SizeY					= mSize.Height();
	MapHdr.TileSizeX				= mTileSize.Width();
	MapHdr.TileSizeY				= mTileSize.Height();
	MapHdr.LayerCount				= mLayerCount;
	MapHdr.PropertyCount			= mProperties.size();
	MapHdr.ShapeGroupCount			= ShapeGroups.size();
	MapHdr.VirtualObjectTypesCount	= mObjTypes.size();	//! This is only usefull for the Map Editor, to auto add on the load the virtual object types that where used to create the map.

	std::fstream fs ( path.c_str() , std::ios::out | std::ios::binary );

	if ( fs.is_open() ) {
		//! Writes the map header
		fs.write( reinterpret_cast<const char*> ( &MapHdr ), sizeof(sMapHdr) );

		//! Writes the properties of the map
		for ( cMap::PropertiesMap::iterator it = mProperties.begin(); it != mProperties.end(); it++ ) {
			sPropertyHdr tProp;

			memset( tProp.Name, 0, MAP_PROPERTY_SIZE );
			memset( tProp.Value, 0, MAP_PROPERTY_SIZE );

			StrCopy( tProp.Name, (*it).first.c_str(), MAP_PROPERTY_SIZE );
			StrCopy( tProp.Value, (*it).second.c_str(), MAP_PROPERTY_SIZE );

			fs.write( reinterpret_cast<const char*> ( &tProp ), sizeof(sPropertyHdr) );
		}

		//! Writes the shape groups that the map will need and load
		for ( i = 0; i < ShapeGroups.size(); i++ ) {
			sMapShapeGroup tSG;

			memset( tSG.Path, 0, MAP_SHAPEGROUP_PATH_SIZE );

			StrCopy( tSG.Path, ShapeGroups[i].c_str(), MAP_SHAPEGROUP_PATH_SIZE );

			fs.write( reinterpret_cast<const char*> ( &tSG ), sizeof(sMapShapeGroup) );
		}

		//! Writes the names of the virtual object types created in the map editor
		for ( GOTypesList::iterator votit = mObjTypes.begin(); votit != mObjTypes.end(); votit++ ) {
			sVirtualObj tVObjH;

			memset( tVObjH.Name, 0, MAP_PROPERTY_SIZE );

			StrCopy( tVObjH.Name, (*votit).c_str(), MAP_PROPERTY_SIZE );

			fs.write( reinterpret_cast<const char*> ( &tVObjH ), sizeof(sVirtualObj) );
		}

		//! Writes every layer header
		for ( i = 0; i < mLayerCount; i++ ) {
			tLayer = mLayers[i];
			sLayerHdr tLayerH;

			memset( tLayerH.Name, 0, LAYER_NAME_SIZE );

			StrCopy( tLayerH.Name, tLayer->Name().c_str(), LAYER_NAME_SIZE );

			tLayerH.Type			= tLayer->Type();
			tLayerH.Flags			= tLayer->Flags();
			tLayerH.OffsetX			= tLayer->Offset().x;
			tLayerH.OffsetY			= tLayer->Offset().y;

			if ( MAP_LAYER_OBJECT == tLayerH.Type )
				tLayerH.ObjectCount = reinterpret_cast<cObjectLayer*> ( tLayer )->GetObjectCount();
			else
				tLayerH.ObjectCount		= 0;

			cLayer::PropertiesMap& tLayerProp = tLayer->GetProperties();

			tLayerH.PropertyCount	= tLayerProp.size();

			//! Writes the layer header
			fs.write( reinterpret_cast<const char*> ( &tLayerH ), sizeof(sLayerHdr) );

			//! Writes the properties of the current layer
			for ( cLayer::PropertiesMap::iterator lit = tLayerProp.begin(); lit != tLayerProp.end(); lit++ ) {
				sPropertyHdr tProp;

				memset( tProp.Name, 0, MAP_PROPERTY_SIZE );
				memset( tProp.Value, 0, MAP_PROPERTY_SIZE );

				StrCopy( tProp.Name, (*lit).first.c_str(), MAP_PROPERTY_SIZE );
				StrCopy( tProp.Value, (*lit).second.c_str(), MAP_PROPERTY_SIZE );
			}
		}

		bool ThereIsTiled = false;

		for ( i = 0; i < mLayerCount; i++ ) {
			if ( NULL != mLayers[i] && mLayers[i]->Type() == MAP_LAYER_TILED ) {
				ThereIsTiled = true;
			}
		}

		//! This method is slow, but allows to save big maps with little space needed, i'll add an alternative save method ( just plain layer -> tile object saving )
		Int32 x, y;
		Uint32 tReadFlag = 0, z;
		cTileLayer * tTLayer;
		cGameObject * tObj;

		cGameObject * tObjects[ mLayerCount ];

		if ( ThereIsTiled ) {
			//! First we save the tiled layers.
			for ( y = 0; y < mSize.y; y++ ) {
				for ( x = 0; x < mSize.x; x++ ) {
					//! Reset Layer Read Flags and temporal objects
					tReadFlag		= 0;

					for ( z = 0; z < mLayerCount; z++ )
						tObjects[z] = NULL;

					//! Look at every layer if it's some data on the current tile, in that case it will write a bit flag to
					//! inform that it's an object on the current tile layer, and it will store a temporal reference to the
					//! object to write layer the object header information
					for ( i = 0; i < mLayerCount; i++ ) {
						tLayer = mLayers[i];

						if ( NULL != tLayer && tLayer->Type() == MAP_LAYER_TILED ) {
							tTLayer = reinterpret_cast<cTileLayer*> ( tLayer );

							tObj = tTLayer->GetGameObject( eeVector2i( x, y ) );

							if ( NULL != tObj ) {
								tReadFlag |= 1 << i;

								tObjects[i] = tObj;
							}
						}
					}

					//! Writes the current tile flags
					fs.write( reinterpret_cast<const char*> ( &tReadFlag ), sizeof(Uint32) );

					//! Writes every game object header corresponding to this tile
					for ( i = 0; i < mLayerCount; i++ ) {
						if ( tReadFlag & ( 1 << i ) ) {
							tObj = tObjects[i];

							sMapTileGOHdr tTGOHdr;

							//! The DataId should be the Shape hash name ( at least in the cases of type Shape, ShapeEx and Sprite.
							tTGOHdr.Id		= tObj->DataId();

							//! If the object type is virtual, means that the real type is stored elsewhere.
							if ( tObj->Type() != GAMEOBJECT_TYPE_VIRTUAL ) {
								tTGOHdr.Type	= tObj->Type();
							} else {
								cGameObjectVirtual * tObjV = reinterpret_cast<cGameObjectVirtual*> ( tObj );

								tTGOHdr.Type	= tObjV->RealType();
							}

							tTGOHdr.Flags	= tObj->Flags();

							fs.write( reinterpret_cast<const char*> ( &tTGOHdr ), sizeof(sMapTileGOHdr) );
						}
					}
				}
			}
		}

		//! Then we save the Object layers.
		cObjectLayer * tOLayer;

		for ( i = 0; i < mLayerCount; i++ ) {
			tLayer = mLayers[i];

			if ( NULL != tLayer && tLayer->Type() == MAP_LAYER_OBJECT ) {
				tOLayer = reinterpret_cast<cObjectLayer*> ( tLayer );

				cObjectLayer::ObjList ObjList = tOLayer->GetObjectList();

				for ( cObjectLayer::ObjList::iterator MapObjIt = ObjList.begin(); MapObjIt != ObjList.end(); MapObjIt++ ) {
					tObj = (*MapObjIt);

					sMapObjGOHdr tOGOHdr;

					//! The DataId should be the Shape hash name ( at least in the cases of type Shape, ShapeEx and Sprite.
					tOGOHdr.Id		= tObj->DataId();

					//! If the object type is virtual, means that the real type is stored elsewhere.
					if ( tObj->Type() != GAMEOBJECT_TYPE_VIRTUAL ) {
						tOGOHdr.Type	= tObj->Type();
					} else {
						cGameObjectVirtual * tObjV = reinterpret_cast<cGameObjectVirtual*> ( tObj );

						tOGOHdr.Type	= tObjV->RealType();
					}

					tOGOHdr.Flags	= tObj->Flags();

					tOGOHdr.PosX	= (Int32)tObj->Pos().x;

					tOGOHdr.PosY	= (Int32)tObj->Pos().y;

					fs.write( reinterpret_cast<const char*> ( &tOGOHdr ), sizeof(sMapObjGOHdr) );
				}
			}
		}

		fs.close();
	}
}

std::vector<std::string> cMap::GetShapeGroups() {
	cShapeGroupManager * SGM = cShapeGroupManager::instance();
	std::list<cShapeGroup*>& Res = SGM->GetResources();

	std::vector<std::string> items;

	//! Ugly ugly ugly, but i don't see another way
	Uint32 Restricted1 = MakeHash( std::string( "global" ) );
	Uint32 Restricted2 = MakeHash( UI::cUIThemeManager::instance()->DefaultTheme()->ShapeGroup()->Name() );

	for ( std::list<cShapeGroup*>::iterator it = Res.begin(); it != Res.end(); it++ ) {
		if ( (*it)->Id() != Restricted1 && (*it)->Id() != Restricted2 )
			items.push_back( (*it)->Path() );
	}

	return items;
}

}}