#include <deque>
#include <eepp/graphics/renderer/renderer.hpp>
#include <eepp/ui/uilinearlayout.hpp>
#include <eepp/ui/uipushbutton.hpp>
#include <eepp/ui/uiscenenode.hpp>
#include <eepp/ui/uiscrollbar.hpp>
#include <eepp/ui/uitreeview.hpp>

namespace EE { namespace UI {

UITreeView* UITreeView::New() {
	return eeNew( UITreeView, () );
}

UITreeView::UITreeView() :
	UIAbstractTableView( "treeview" ),
	mIndentWidth( PixelDensity::dpToPx( 12 ) ),
	mExpanderIconSize( PixelDensity::dpToPxI( 12 ) ) {
	clipEnable();
	mExpandIcon = getUISceneNode()->findIcon( "tree-expanded" );
	mContractIcon = getUISceneNode()->findIcon( "tree-contracted" );
}

Uint32 UITreeView::getType() const {
	return UI_TYPE_TREEVIEW;
}

bool UITreeView::isType( const Uint32& type ) const {
	return UITreeView::getType() == type ? true : UIAbstractTableView::isType( type );
}

UITreeView::MetadataForIndex& UITreeView::getIndexMetadata( const ModelIndex& index ) const {
	eeASSERT( index.isValid() );
	auto it = mViewMetadata.find( index.data() );
	if ( it != mViewMetadata.end() )
		return it->second;
	auto newMetadata = MetadataForIndex();
	mViewMetadata.insert( {index.data(), std::move( newMetadata )} );
	return mViewMetadata[index.data()];
}

template <typename Callback> void UITreeView::traverseTree( Callback callback ) const {
	if ( !getModel() )
		return;
	auto& model = *getModel();
	int indentLevel = 0;
	Float yOffset = getHeaderHeight();
	int rowIndex = -1;
	std::function<IterationDecision( const ModelIndex& )> traverseIndex =
		[&]( const ModelIndex& index ) {
			if ( index.isValid() ) {
				auto& metadata = getIndexMetadata( index );
				rowIndex++;
				IterationDecision decision = callback( rowIndex, index, indentLevel, yOffset );
				if ( decision == IterationDecision::Break || decision == IterationDecision::Stop )
					return decision;
				yOffset += getRowHeight();
				if ( !metadata.open ) {
					return IterationDecision::Continue;
				}
			}
			if ( indentLevel >= 0 && !index.isValid() )
				return IterationDecision::Continue;
			++indentLevel;
			int rowCount = model.rowCount( index );
			for ( int i = 0; i < rowCount; ++i ) {
				IterationDecision decision =
					traverseIndex( model.index( i, model.treeColumn(), index ) );
				if ( decision == IterationDecision::Break || decision == IterationDecision::Stop )
					return decision;
			}
			--indentLevel;
			return IterationDecision::Continue;
		};
	int rootCount = model.rowCount();
	for ( int rootIndex = 0; rootIndex < rootCount; ++rootIndex ) {
		IterationDecision decision =
			traverseIndex( model.index( rootIndex, model.treeColumn(), ModelIndex() ) );
		if ( decision == IterationDecision::Break || decision == IterationDecision::Stop )
			break;
	}
}

void UITreeView::createOrUpdateColumns() {
	if ( !getModel() )
		return;
	updateContentSize();
	UIAbstractTableView::createOrUpdateColumns();
}

size_t UITreeView::getItemCount() const {
	size_t count = 0;
	traverseTree( [&]( const int&, const ModelIndex&, const size_t&, const Float& ) {
		count++;
		return IterationDecision::Continue;
	} );
	return count;
}

void UITreeView::onColumnSizeChange( const size_t& ) {
	updateContentSize();
}

void UITreeView::updateContentSize() {
	Sizef oldSize( mContentSize );
	mContentSize = UIAbstractTableView::getContentSize();
	if ( oldSize != mContentSize )
		onContentSizeChange();
}

class UITreeViewCell : public UITableCell {
  public:
	static UITreeViewCell* New() { return eeNew( UITreeViewCell, () ); }

	Uint32 getType() const { return UI_TYPE_TREEVIEW_CELL; }

	bool isType( const Uint32& type ) const {
		return UITreeViewCell::getType() == type ? true : UITableCell::isType( type );
	}

	UIImage* getImage() const { return mImage; }

	Rectf calculatePadding() const {
		Sizef size;
		Rectf autoPadding;
		if ( mFlags & UI_AUTO_PADDING ) {
			autoPadding = makePadding( true, true, true, true );
			if ( autoPadding != Rectf() )
				autoPadding = PixelDensity::dpToPx( autoPadding );
		}
		if ( mPaddingPx.Top > autoPadding.Top )
			autoPadding.Top = mPaddingPx.Top;
		if ( mPaddingPx.Bottom > autoPadding.Bottom )
			autoPadding.Bottom = mPaddingPx.Bottom;
		if ( mPaddingPx.Left > autoPadding.Left )
			autoPadding.Left = mPaddingPx.Left;
		if ( mPaddingPx.Right > autoPadding.Right )
			autoPadding.Right = mPaddingPx.Right;
		autoPadding.Left += mIndent;
		return autoPadding;
	}

	void setIndentation( const Float& indent ) {
		if ( mIndent != indent ) {
			mIndent = indent;
			updateLayout();
		}
	}

	const Float& getIndentation() const { return mIndent; }

  protected:
	mutable UIImage* mImage{nullptr};
	Float mIndent{0};

	UITreeViewCell() : UITableCell( "treeview::cell" ) {
		mTextBox->setElementTag( mTag + "::text" );
		mIcon->setElementTag( mTag + "::icon" );
		mInnerWidgetOrientation = InnerWidgetOrientation::Left;
		auto cb = [&]( const Event* ) { updateLayout(); };
		mImage = UIImage::NewWithTag( mTag + "::expander" );
		mImage->setScaleType( UIScaleType::FitInside )
			->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::Fixed )
			->setFlags( UI_VALIGN_CENTER | UI_HALIGN_CENTER )
			->setParent( const_cast<UITreeViewCell*>( this ) )
			->setVisible( false )
			->setEnabled( false );
		mImage->addEventListener( Event::OnPaddingChange, cb );
		mImage->addEventListener( Event::OnMarginChange, cb );
		mImage->addEventListener( Event::OnSizeChange, cb );
		mImage->addEventListener( Event::OnVisibleChange, cb );
	}

	virtual UIWidget* getExtraInnerWidget() const { return mImage; }
};

UIWidget* UITreeView::createCell( UIWidget* rowWidget, const ModelIndex& index ) {
	UITableCell* widget = index.column() == (Int64)getModel()->treeColumn() ? UITreeViewCell::New()
																			: UITableCell::New();
	widget->setParent( rowWidget );
	widget->unsetFlags( UI_AUTO_SIZE );
	widget->clipEnable();
	widget->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::Fixed );
	widget->setTextAlign( UI_HALIGN_LEFT );
	widget->setCurIndex( index );
	if ( index.column() == (Int64)getModel()->treeColumn() ) {
		widget->addEventListener( Event::MouseDoubleClick, [&]( const Event* event ) {
			auto mouseEvent = static_cast<const MouseEvent*>( event );
			auto idx = mouseEvent->getNode()->getParent()->asType<UITableRow>()->getCurIndex();
			if ( mouseEvent->getFlags() & EE_BUTTON_LMASK ) {
				if ( getModel()->rowCount( idx ) ) {
					auto& data = getIndexMetadata( idx );
					data.open = !data.open;
					createOrUpdateColumns();
					onOpenTreeModelIndex( idx, data.open );
				} else {
					onOpenModelIndex( idx );
				}
			}
		} );
		widget->addEventListener( Event::MouseClick, [&]( const Event* event ) {
			auto mouseEvent = static_cast<const MouseEvent*>( event );
			UIWidget* icon = mouseEvent->getNode()->asType<UIPushButton>()->getExtraInnerWidget();
			if ( icon ) {
				Vector2f pos( icon->convertToNodeSpace( mouseEvent->getPosition().asFloat() ) );
				if ( pos >= Vector2f::Zero && pos <= icon->getPixelsSize() ) {
					auto idx =
						mouseEvent->getNode()->getParent()->asType<UITableRow>()->getCurIndex();
					if ( getModel()->rowCount( idx ) ) {
						auto& data = getIndexMetadata( idx );
						data.open = !data.open;
						createOrUpdateColumns();
						onOpenTreeModelIndex( idx, data.open );
					}
				}
			}
		} );
	}
	return widget;
}

UIWidget* UITreeView::updateCell( const int& rowIndex, const ModelIndex& index,
								  const size_t& indentLevel, const Float& yOffset ) {
	if ( rowIndex >= (int)mWidgets.size() )
		mWidgets.resize( rowIndex + 1 );
	auto* widget = mWidgets[rowIndex][index.column()];
	if ( !widget ) {
		UIWidget* rowWidget = updateRow( rowIndex, index, yOffset );
		widget = createCell( rowWidget, index );
		mWidgets[rowIndex][index.column()] = widget;
		widget->reloadStyle( true, true, true );
	}
	widget->setPixelsSize( columnData( index.column() ).width, getRowHeight() );
	widget->setPixelsPosition( {getColumnPosition( index.column() ).x, 0} );

	if ( widget->isType( UI_TYPE_TABLECELL ) ) {
		UITableCell* cell = widget->asType<UITableCell>();
		cell->setCurIndex( index );

		Variant txt( getModel()->data( index, Model::Role::Display ) );
		if ( txt.isValid() ) {
			if ( txt.is( Variant::Type::String ) )
				cell->setText( txt.asString() );
			else if ( txt.is( Variant::Type::cstr ) )
				cell->setText( txt.asCStr() );
		}

		bool hasChilds = false;

		if ( widget->isType( UI_TYPE_TREEVIEW_CELL ) ) {
			UITreeViewCell* cell = widget->asType<UITreeViewCell>();
			UIImage* image = widget->asType<UITreeViewCell>()->getImage();

			Float minIndent =
				!mExpandersAsIcons
					? eemax( mExpandIcon->getSize( mExpanderIconSize )->getPixelsSize().getWidth(),
							 mContractIcon->getSize( mExpanderIconSize )
								 ->getPixelsSize()
								 .getWidth() ) +
						  PixelDensity::dpToPx( image->getLayoutMargin().Right )
					: 0;

			if ( index.column() == (Int64)getModel()->treeColumn() )
				cell->setIndentation( minIndent + getIndentWidth() * indentLevel );

			hasChilds = getModel()->rowCount( index ) > 0;

			if ( hasChilds ) {
				UIIcon* icon = getIndexMetadata( index ).open ? mExpandIcon : mContractIcon;
				Drawable* drawable = icon->getSize( mExpanderIconSize );

				image->setVisible( true );
				image->setPixelsSize( drawable->getPixelsSize() );
				image->setDrawable( drawable );
				if ( !mExpandersAsIcons ) {
					cell->setIndentation( cell->getIndentation() -
										  image->getPixelsSize().getWidth() -
										  PixelDensity::dpToPx( image->getLayoutMargin().Right ) );
				}
			} else {
				image->setVisible( false );
			}
		}

		if ( hasChilds && mExpandersAsIcons ) {
			cell->getIcon()->setVisible( false );
			return widget;
		}

		bool isVisible = false;
		Variant icon( getModel()->data( index, Model::Role::Icon ) );
		if ( icon.is( Variant::Type::Drawable ) && icon.asDrawable() ) {
			isVisible = true;
			cell->setIcon( icon.asDrawable() );
		} else if ( icon.is( Variant::Type::Icon ) && icon.asIcon() ) {
			isVisible = true;
			cell->setIcon( icon.asIcon()->getSize( mIconSize ) );
		}
		cell->getIcon()->setVisible( isVisible );
	}

	return widget;
}

const Float& UITreeView::getIndentWidth() const {
	return mIndentWidth;
}

void UITreeView::setIndentWidth( const Float& indentWidth ) {
	if ( mIndentWidth != indentWidth ) {
		mIndentWidth = indentWidth;
		createOrUpdateColumns();
	}
}

Sizef UITreeView::getContentSize() const {
	return mContentSize;
}

void UITreeView::drawChilds() {
	int realIndex = 0;

	traverseTree( [&]( const int&, const ModelIndex& index, const size_t& indentLevel,
					   const Float& yOffset ) {
		if ( yOffset - mScrollOffset.y > mSize.getHeight() )
			return IterationDecision::Stop;
		if ( yOffset - mScrollOffset.y + getRowHeight() < 0 )
			return IterationDecision::Continue;
		for ( size_t colIndex = 0; colIndex < getModel()->columnCount(); colIndex++ ) {
			if ( columnData( colIndex ).visible ) {
				if ( (Int64)colIndex != index.column() ) {
					updateCell( realIndex,
								getModel()->index( index.row(), colIndex, index.parent() ),
								indentLevel, yOffset );
				} else {
					updateCell( realIndex, index, indentLevel, yOffset );
				}
			}
		}
		updateRow( realIndex, index, yOffset )->nodeDraw();
		realIndex++;
		return IterationDecision::Continue;
	} );

	if ( mHeader && mHeader->isVisible() )
		mHeader->nodeDraw();
	if ( mHScroll->isVisible() )
		mHScroll->nodeDraw();
	if ( mVScroll->isVisible() )
		mVScroll->nodeDraw();
}

Node* UITreeView::overFind( const Vector2f& point ) {
	mUISceneNode->setIsLoading( true );

	Node* pOver = NULL;
	if ( mEnabled && mVisible ) {
		updateWorldPolygon();
		if ( mWorldBounds.contains( point ) && mPoly.pointInside( point ) ) {
			writeNodeFlag( NODE_FLAG_MOUSEOVER_ME_OR_CHILD, 1 );
			mSceneNode->addMouseOverNode( this );
			if ( mHScroll->isVisible() && ( pOver = mHScroll->overFind( point ) ) )
				return pOver;
			if ( mVScroll->isVisible() && ( pOver = mVScroll->overFind( point ) ) )
				return pOver;
			if ( mHeader && ( pOver = mHeader->overFind( point ) ) )
				return pOver;
			int realIndex = 0;
			traverseTree(
				[&, point]( int, const ModelIndex& index, const size_t&, const Float& yOffset ) {
					if ( yOffset - mScrollOffset.y > mSize.getHeight() )
						return IterationDecision::Stop;
					if ( yOffset - mScrollOffset.y + getRowHeight() < 0 )
						return IterationDecision::Continue;
					pOver = updateRow( realIndex, index, yOffset )->overFind( point );
					realIndex++;
					if ( pOver )
						return IterationDecision::Stop;
					return IterationDecision::Continue;
				} );
			if ( !pOver )
				pOver = this;
		}
	}

	mUISceneNode->setIsLoading( false );

	return pOver;
}

bool UITreeView::isExpanded( const ModelIndex& index ) const {
	return getIndexMetadata( index ).open;
}

void UITreeView::setAllExpanded( const ModelIndex& index, bool expanded ) {
	Model& model = *getModel();
	size_t count = model.rowCount( index );
	for ( size_t i = 0; i < count; i++ ) {
		auto curIndex = model.index( i, model.treeColumn(), index );
		getIndexMetadata( curIndex ).open = expanded;
		if ( model.rowCount( curIndex ) > 0 )
			setAllExpanded( curIndex, expanded );
	}

	createOrUpdateColumns();
}

void UITreeView::expandAll( const ModelIndex& index ) {
	if ( !getModel() )
		return;
	setAllExpanded( index, true );
	createOrUpdateColumns();
}

void UITreeView::contractAll( const ModelIndex& index ) {
	if ( !getModel() )
		return;
	setAllExpanded( index, false );
	createOrUpdateColumns();
}

UIIcon* UITreeView::getExpandIcon() const {
	return mExpandIcon;
}

void UITreeView::setExpandedIcon( UIIcon* expandIcon ) {
	if ( mExpandIcon != expandIcon ) {
		mExpandIcon = expandIcon;
		createOrUpdateColumns();
	}
}

void UITreeView::setExpandedIcon( const std::string& expandIcon ) {
	setExpandedIcon( mUISceneNode->findIcon( expandIcon ) );
}

UIIcon* UITreeView::getContractIcon() const {
	return mContractIcon;
}

void UITreeView::setContractedIcon( UIIcon* contractIcon ) {
	if ( mContractIcon != contractIcon ) {
		mContractIcon = contractIcon;
		createOrUpdateColumns();
	}
}

void UITreeView::setContractedIcon( const std::string& contractIcon ) {
	setContractedIcon( mUISceneNode->findIcon( contractIcon ) );
}

bool UITreeView::getExpandersAsIcons() const {
	return mExpandersAsIcons;
}

void UITreeView::setExpandersAsIcons( bool expandersAsIcons ) {
	mExpandersAsIcons = expandersAsIcons;
}

Float UITreeView::getMaxColumnContentWidth( const size_t& colIndex ) {
	Float lWidth = 0;
	getUISceneNode()->setIsLoading( true );
	traverseTree( [&, colIndex]( const int&, const ModelIndex& index, const size_t& indentLevel,
								 const Float& yOffset ) {
		UIWidget* widget = updateCell(
			0, getModel()->index( index.row(), colIndex, index.parent() ), indentLevel, yOffset );
		if ( widget->isType( UI_TYPE_PUSHBUTTON ) ) {
			Float w = widget->asType<UIPushButton>()->getContentSize().getWidth();
			if ( w > lWidth )
				lWidth = w;
		}
		return IterationDecision::Continue;
	} );
	getUISceneNode()->setIsLoading( false );
	return lWidth;
}

const size_t& UITreeView::getExpanderIconSize() const {
	return mExpanderIconSize;
}

void UITreeView::setExpanderIconSize( const size_t& expanderSize ) {
	mExpanderIconSize = expanderSize;
}

Uint32 UITreeView::onKeyDown( const KeyEvent& event ) {
	auto curIndex = getSelection().first();

	switch ( event.getKeyCode() ) {
		case KEY_PAGEUP: {
			int pageSize = eefloor( getVisibleArea().getHeight() / getRowHeight() ) - 1;
			std::deque<std::pair<ModelIndex, Float>> deque;
			Float curY;
			traverseTree(
				[&]( const int&, const ModelIndex& index, const size_t&, const Float& offsetY ) {
					deque.push_back( {index, offsetY} );
					if ( (int)deque.size() > pageSize )
						deque.pop_front();
					if ( index == curIndex )
						return IterationDecision::Break;
					return IterationDecision::Continue;
				} );
			curY = deque.front().second - getHeaderHeight();
			getSelection().set( deque.front().first );
			scrollToPosition(
				{{mScrollOffset.x, curY},
				 {columnData( deque.front().first.column() ).width, getRowHeight()}} );
			return 1;
		}
		case KEY_PAGEDOWN: {
			int pageSize = eefloor( getVisibleArea().getHeight() / getRowHeight() ) - 1;
			int counted = 0;
			bool foundStart = false;
			bool resultFound = false;
			ModelIndex foundIndex;
			Float curY;
			Float lastOffsetY;
			ModelIndex lastIndex;
			traverseTree(
				[&]( const int&, const ModelIndex& index, const size_t&, const Float& offsetY ) {
					if ( index == curIndex ) {
						foundStart = true;
					} else if ( foundStart ) {
						counted++;
						if ( counted == pageSize ) {
							foundIndex = index;
							curY = offsetY;
							resultFound = true;
							return IterationDecision::Break;
						}
					}
					lastOffsetY = offsetY;
					lastIndex = index;
					return IterationDecision::Continue;
				} );
			if ( !resultFound ) {
				foundIndex = lastIndex;
				curY = lastOffsetY;
			}
			curY += getRowHeight();
			getSelection().set( foundIndex );
			scrollToPosition( {{mScrollOffset.x, curY},
							   {columnData( foundIndex.column() ).width, getRowHeight()}} );
			return 1;
		}
		case KEY_UP: {
			ModelIndex prevIndex;
			ModelIndex foundIndex;
			Float curY = 0;
			traverseTree(
				[&]( const int&, const ModelIndex& index, const size_t&, const Float& offsetY ) {
					if ( index == curIndex ) {
						foundIndex = prevIndex;
						curY = offsetY;
						return IterationDecision::Break;
					}
					prevIndex = index;
					return IterationDecision::Continue;
				} );
			if ( foundIndex.isValid() ) {
				getSelection().set( foundIndex );
				if ( curY < mScrollOffset.y + getHeaderHeight() + getRowHeight() ||
					 curY > mScrollOffset.y + getPixelsSize().getHeight() - mPaddingPx.Top -
								mPaddingPx.Bottom - getRowHeight() ) {
					curY -= getHeaderHeight() + getRowHeight();
					mVScroll->setValue( eemin<Float>(
						1.f, eemax<Float>( 0.f, curY / getScrollableArea().getHeight() ) ) );
				}
			}
			return 1;
		}
		case KEY_DOWN: {
			ModelIndex prevIndex;
			ModelIndex foundIndex;
			Float curY = 0;
			traverseTree(
				[&]( const int&, const ModelIndex& index, const size_t&, const Float& offsetY ) {
					if ( prevIndex == curIndex ) {
						foundIndex = index;
						curY = offsetY;
						return IterationDecision::Break;
					}
					prevIndex = index;
					return IterationDecision::Continue;
				} );
			if ( foundIndex.isValid() ) {
				getSelection().set( foundIndex );
				if ( curY < mScrollOffset.y ||
					 curY > mScrollOffset.y + getPixelsSize().getHeight() - mPaddingPx.Top -
								mPaddingPx.Bottom - getRowHeight() ) {
					curY -=
						eefloor( getVisibleArea().getHeight() / getRowHeight() ) * getRowHeight() -
						getRowHeight();
					mVScroll->setValue(
						eemin<Float>( 1.f, curY / getScrollableArea().getHeight() ) );
				}
			}
			return 1;
		}
		case KEY_END: {
			scrollToBottom();
			ModelIndex lastIndex;
			traverseTree( [&]( const int&, const ModelIndex& index, const size_t&, const Float& ) {
				lastIndex = index;
				return IterationDecision::Continue;
			} );
			getSelection().set( lastIndex );
			return 1;
		}
		case KEY_HOME: {
			scrollToTop();
			getSelection().set( getModel()->index( 0, 0 ) );
			return 1;
		}
		case KEY_RIGHT: {
			if ( curIndex.isValid() && getModel()->rowCount( curIndex ) ) {
				auto& metadata = getIndexMetadata( curIndex );
				if ( !metadata.open ) {
					metadata.open = true;
					createOrUpdateColumns();
					return 0;
				}
				getSelection().set( getModel()->index( 0, getModel()->treeColumn(), curIndex ) );
			}
			return 1;
		}
		case KEY_LEFT: {
			if ( curIndex.isValid() && getModel()->rowCount( curIndex ) ) {
				auto& metadata = getIndexMetadata( curIndex );
				if ( metadata.open ) {
					metadata.open = false;
					createOrUpdateColumns();
					return 0;
				}
			}
			if ( curIndex.isValid() && curIndex.parent().isValid() ) {
				getSelection().set( curIndex.parent() );
				return 0;
			}
			return 1;
		}
		case KEY_RETURN:
		case KEY_SPACE: {
			if ( curIndex.isValid() ) {
				if ( getModel()->rowCount( curIndex ) ) {
					auto& metadata = getIndexMetadata( curIndex );
					metadata.open = !metadata.open;
					createOrUpdateColumns();
				} else {
					onOpenModelIndex( curIndex );
				}
			}
			return 1;
		}
		default:
			break;
	}
	return UIAbstractTableView::onKeyDown( event );
}

void UITreeView::onOpenTreeModelIndex( const ModelIndex& index, bool open ) {
	ModelEvent event( getModel(), index, this,
					  open ? ModelEventType::OpenTree : ModelEventType::CloseTree );
	sendEvent( &event );
}

}} // namespace EE::UI
