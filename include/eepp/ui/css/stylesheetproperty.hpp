#ifndef EE_UI_CSS_STYLESHEETPROPERTY_HPP
#define EE_UI_CSS_STYLESHEETPROPERTY_HPP

#include <string>
#include <map>
#include <eepp/config.hpp>
#include <eepp/core/string.hpp>
#include <eepp/system/time.hpp>
#include <eepp/system/color.hpp>
#include <eepp/math/rect.hpp>
#include <eepp/math/originpoint.hpp>
#include <eepp/math/ease.hpp>
#include <eepp/graphics/blendmode.hpp>

using namespace EE::System;
using namespace EE::Math;
using namespace EE::Graphics;

namespace EE { namespace UI { namespace CSS {

class EE_API StyleSheetProperty {
	public:
		StyleSheetProperty();

		explicit StyleSheetProperty( const std::string& name, const std::string& value );

		explicit StyleSheetProperty( const std::string& name, const std::string& value, const Uint32& specificity, const bool& isVolatile = false );

		const std::string& getName() const;

		const Uint32& getNameHash() const;

		const std::string& getValue() const;

		const std::string& value() const;

		const Uint32& getSpecificity() const;

		void setSpecificity( const Uint32& specificity );

		bool isEmpty() const;

		void setName(const std::string & name);

		void setValue(const std::string & value);

		const bool& isVolatile() const;

		void setVolatile( const bool& isVolatile );

		bool operator==( const StyleSheetProperty& property );

		std::string asString( const std::string& defaultValue = "" ) const;

		template<typename Type>
		Type asType( Type defaultValue ) const {
			Type val = defaultValue;
			return String::fromString<Type>( val, mValue ) ? val : defaultValue;
		}

		int asInt( int defaultValue = 0 ) const;

		unsigned int asUint( unsigned int defaultValue = 0 ) const;

		double asDouble( double defaultValue = 0 ) const;

		float asFloat( float defaultValue = 0 ) const;

		long long asLlong( long long defaultValue = 0)  const;

		unsigned long long asUllong( unsigned long long defaultValue = 0 ) const;

		bool asBool( bool defaultValue = false ) const;

		Color asColor() const;

		Float asDpDimension( const std::string& defaultValue = "" ) const;

		int asDpDimensionI( const std::string& defaultValue = "" ) const;

		Uint32 asDpDimensionUint( const std::string& defaultValue = "" ) const;

		OriginPoint asOriginPoint() const;

		BlendMode asBlendMode() const;

		Vector2f asVector2f( const Vector2f& defaultValue = Vector2f::Zero ) const;

		Vector2i asVector2i( const Vector2i& defaultValue = Vector2i::Zero ) const;

		Sizef asSizef( const Sizef& defaultValue = Sizef::Zero ) const;

		Sizei asSizei( const Sizei& defaultValue = Sizei::Zero ) const;

		Rect asRect( const Rect& defaultValue = Rect() ) const;

		Rectf asRectf( const Rectf& defaultValue = Rectf() ) const;

		Uint32 asFontStyle() const;

		Time asTime( const Time& defaultTime = Seconds(0) );

		Ease::Interpolation asInterpolation( const Ease::Interpolation& defaultInterpolation = Ease::Linear );
	protected:
		std::string mName;
		Uint32 mNameHash;
		std::string mValue;
		Uint32 mSpecificity;
		bool mVolatile;
		bool mImportant;

		void checkImportant();
};

typedef std::map<std::string, StyleSheetProperty> StyleSheetProperties;

}}}

#endif
