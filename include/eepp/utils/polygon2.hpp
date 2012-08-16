#ifndef EE_UTILSPOLIGON2_H
#define EE_UTILSPOLIGON2_H

#include <eepp/utils/triangle2.hpp>
#include <eepp/utils/quad2.hpp>
#include <eepp/utils/rect.hpp>

namespace EE { namespace Utils {

template <typename T>
class Polygon2 {
	public:
		Polygon2();
		~Polygon2();
		Polygon2( const Triangle2<T>& fromTrig );
		Polygon2( const Quad2<T>& fromQuad );
		Polygon2( const tRECT<T>& fromRect );
		Polygon2( const std::vector< Vector2<T> >& theVecs );

		Uint32 PushBack( const Vector2<T>& V );
		void PopBack();

		void Reset();

		const Vector2<T>& operator[] ( const Uint32& Pos ) const;

		std::size_t Size() const;

		Vector2<T> Position() { return Vector2<T>(cOffsetX, cOffsetY); };
		T X() const { return cOffsetX; };
		T Y() const { return cOffsetY; };

		void Position( const Vector2<T>& V ) { cOffsetX = V.x; cOffsetY = V.y; }
		T X( const T& x ) { cOffsetX = x; }
		T Y( const T& y ) { cOffsetY = y; }

		void Rotate( const T& Angle, const Vector2<T>& Center );

		void Scale( const T& scale, const Vector2<T>& Center );
	private:
		std::vector< Vector2<T> > Vector;
		T cOffsetX, cOffsetY;
};

template <typename T>
Polygon2<T>::Polygon2() : cOffsetX(0), cOffsetY(0) {
	Reset();
}

template <typename T>
Polygon2<T>::Polygon2( const std::vector< Vector2<T> >& theVecs ) : cOffsetX(0), cOffsetY(0) {
	for (Uint32 i = 0; i < theVecs.size(); i++)
		PushBack ( theVecs[i] );
}

template <typename T>
Polygon2<T>::Polygon2( const Triangle2<T>& fromTrig ) : cOffsetX(0), cOffsetY(0) {
	for (Uint8 i = 0; i < 3; i++)
		PushBack ( fromTrig.V[i] );
}

template <typename T>
Polygon2<T>::Polygon2( const Quad2<T>& fromQuad ) : cOffsetX(0), cOffsetY(0) {
	for (Uint8 i = 0; i < 4; i++)
		PushBack ( fromQuad.V[i] );
}

template<typename T>
Polygon2<T>::Polygon2( const tRECT<T>& fromRect ) : cOffsetX(0), cOffsetY(0) {
	Vector.push_back( Vector2<T>( fromRect.Left, fromRect.Top ) );
	Vector.push_back( Vector2<T>( fromRect.Left, fromRect.Bottom ) );
	Vector.push_back( Vector2<T>( fromRect.Right, fromRect.Bottom ) );
	Vector.push_back( Vector2<T>( fromRect.Right, fromRect.Top ) );
}

template <typename T>
Polygon2<T>::~Polygon2() {
	Reset();
}

template <typename T>
void Polygon2<T>::Reset() {
	Vector.clear();
}

template <typename T>
Uint32 Polygon2<T>::PushBack( const Vector2<T>& V ) {
	Vector.push_back( V );
	return (Uint32)Vector.size() - 1;
}

template <typename T>
void Polygon2<T>::PopBack() {
	Vector.pop_back();
}

template <typename T>
const Vector2<T>& Polygon2<T>::operator[] ( const Uint32& Pos ) const {
	if ( Vector.size() > 0 && Pos < Vector.size() )
		return Vector[Pos];
	return Vector[0];
}

template <typename T>
std::size_t Polygon2<T>::Size() const {
	return Vector.size();
}

template <typename T>
void Polygon2<T>::Rotate( const T& Angle, const Vector2<T>& Center ) {
	if ( Angle == 0.f )
		return;

	for ( eeUint i = 0; i < Vector.size(); i++ )
		Vector[ i ].RotateVectorCentered( Angle, Center );
}

template <typename T>
void Polygon2<T>::Scale( const T& scale, const Vector2<T>& Center ) {
	if ( scale == 1.0f )
		return;

	for ( Uint32 i = 0; i < Vector.size(); i++ ) {
		if ( Vector[i].x < Center.x )
			Vector[i].x = Center.x - eeabs( Center.x - Vector[i].x ) * scale;
		else
			Vector[i].x = Center.x + eeabs( Center.x - Vector[i].x ) * scale;

		if ( Vector[i].y < Center.y )
			Vector[i].y = Center.y - eeabs( Center.y - Vector[i].y ) * scale;
		else
			Vector[i].y = Center.y + eeabs( Center.y - Vector[i].y ) * scale;
	}
}

typedef Polygon2<eeFloat> eePolygon2f;

}}

#endif