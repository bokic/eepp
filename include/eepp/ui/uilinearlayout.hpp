#ifndef UI_UILINEARLAYOUT_HPP
#define UI_UILINEARLAYOUT_HPP

#include <eepp/ui/uilayout.hpp>

namespace EE { namespace UI {

class EE_API UILinearLayout : public UILayout {
  public:
	static UILinearLayout* New();

	static UILinearLayout* NewVertical();

	static UILinearLayout* NewHorizontal();

	UILinearLayout();

	virtual Uint32 getType() const;

	virtual bool isType( const Uint32& type ) const;

	UIOrientation getOrientation() const;

	UILinearLayout* setOrientation( const UIOrientation& getOrientation );

	UILinearLayout* add( UIWidget* widget );

	virtual bool applyProperty( const StyleSheetProperty& attribute );

	virtual std::string getPropertyString( const PropertyDefinition* propertyDef,
										   const Uint32& propertyIndex = 0 );

	void updateLayout();

	bool isPacking() const;

  protected:
	UIOrientation mOrientation;
	bool mPacking;

	virtual Uint32 onMessage( const NodeMessage* Msg );

	void packVertical();

	void packHorizontal();

	Sizei getTotalUsedSize();
};

}} // namespace EE::UI

#endif
