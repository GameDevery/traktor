#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashTextFormat.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsTextField.h"
#include "Flash/Action/Avm1/Classes/AsTextField_StyleSheet.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

float polarAngle(float x, float y)
{
	if (x == 0.0f && y == 0.0f)
		return 0.0f;
	
	float r = sqrtf(x * x + y * y);

	if (x > 0.0f)
		return asinf(y / r);
	else
		return -asinf(y / r) + PI;
}

void decomposeTransform(const Matrix33& transform, Vector2& outTranslate, Vector2& outScale, float& outRotation)
{
	outTranslate.x = transform.e13;
	outTranslate.y = transform.e23;

	outScale.x = Vector2(transform.e11, transform.e12).length();
	outScale.y = Vector2(transform.e21, transform.e22).length();

	outRotation = polarAngle(transform.e11, transform.e12);
}

Matrix33 composeTransform(const Vector2& translate_, const Vector2& scale_, float rotate_)
{
	return
		translate(translate_.x, translate_.y) *
		scale(scale_.x, scale_.y) *
		rotate(rotate_);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTextField", AsTextField, ActionClass)

AsTextField::AsTextField(ActionContext* context)
:	ActionClass(context, "TextField")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("StyleSheet", ActionValue(new AsTextField_StyleSheet(context)));
	prototype->setMember("addListener", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_addListener)));
	prototype->setMember("getDepth", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_getDepth)));
	prototype->setMember("getFontList", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_getFontList)));
	prototype->setMember("getNewTextFormat", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_getNewTextFormat)));
	prototype->setMember("getTextFormat", ActionValue(
		createPolymorphicFunction(
			context,
			createNativeFunction(context, this, &AsTextField::TextField_getTextFormat_0),
			0,
			createNativeFunction(context, this, &AsTextField::TextField_getTextFormat_2)
		)
	));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_removeListener)));
	prototype->setMember("removeTextField", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_removeTextField)));
	prototype->setMember("replaceSel", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_replaceSel)));
	prototype->setMember("replaceText", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_replaceText)));
	prototype->setMember("setNewTextFormat", ActionValue(createNativeFunction(context, this, &AsTextField::TextField_setNewTextFormat)));
	prototype->setMember("setTextFormat", ActionValue(
		createPolymorphicFunction(
			context,
			createNativeFunction(context, this, &AsTextField::TextField_setTextFormat_0),
			0,
			createNativeFunction(context, this, &AsTextField::TextField_setTextFormat_2)
		)
	));

	prototype->addProperty("_alpha", createNativeFunction(context, this, &AsTextField::TextField_get_alpha), createNativeFunction(context, this, &AsTextField::TextField_set_alpha));
	prototype->addProperty("antiAliasType", createNativeFunction(context, this, &AsTextField::TextField_get_antiAliasType), createNativeFunction(context, this, &AsTextField::TextField_set_antiAliasType));
	prototype->addProperty("autoSize", createNativeFunction(context, this, &AsTextField::TextField_get_autoSize), createNativeFunction(context, this, &AsTextField::TextField_set_autoSize));
	prototype->addProperty("background", createNativeFunction(context, this, &AsTextField::TextField_get_background), createNativeFunction(context, this, &AsTextField::TextField_set_background));
	prototype->addProperty("backgroundColor", createNativeFunction(context, this, &AsTextField::TextField_get_backgroundColor), createNativeFunction(context, this, &AsTextField::TextField_set_backgroundColor));
	prototype->addProperty("border", createNativeFunction(context, this, &AsTextField::TextField_get_border), createNativeFunction(context, this, &AsTextField::TextField_set_border));
	prototype->addProperty("borderColor", createNativeFunction(context, this, &AsTextField::TextField_get_borderColor), createNativeFunction(context, this, &AsTextField::TextField_set_borderColor));
	prototype->addProperty("bottomScroll", createNativeFunction(context, this, &AsTextField::TextField_get_bottomScroll), 0);
	prototype->addProperty("condenseWhite", createNativeFunction(context, this, &AsTextField::TextField_get_condenseWhite), createNativeFunction(context, this, &AsTextField::TextField_set_condenseWhite));
	prototype->addProperty("embedFonts", createNativeFunction(context, this, &AsTextField::TextField_get_embedFonts), createNativeFunction(context, this, &AsTextField::TextField_set_embedFonts));
	prototype->addProperty("filters", createNativeFunction(context, this, &AsTextField::TextField_get_filters), createNativeFunction(context, this, &AsTextField::TextField_set_filters));
	prototype->addProperty("gridFitType", createNativeFunction(context, this, &AsTextField::TextField_get_gridFitType), createNativeFunction(context, this, &AsTextField::TextField_set_gridFitType));
	prototype->addProperty("_height", createNativeFunction(context, this, &AsTextField::TextField_get_height), createNativeFunction(context, this, &AsTextField::TextField_set_height));
	prototype->addProperty("_highquality", createNativeFunction(context, this, &AsTextField::TextField_get_highquality), createNativeFunction(context, this, &AsTextField::TextField_set_highquality));
	prototype->addProperty("hscroll", createNativeFunction(context, this, &AsTextField::TextField_get_hscroll), createNativeFunction(context, this, &AsTextField::TextField_set_hscroll));
	prototype->addProperty("html", createNativeFunction(context, this, &AsTextField::TextField_get_html), createNativeFunction(context, this, &AsTextField::TextField_set_html));
	prototype->addProperty("htmlText", createNativeFunction(context, this, &AsTextField::TextField_get_htmlText), createNativeFunction(context, this, &AsTextField::TextField_set_htmlText));
	prototype->addProperty("length", createNativeFunction(context, this, &AsTextField::TextField_get_length), 0);
	prototype->addProperty("maxChars", createNativeFunction(context, this, &AsTextField::TextField_get_maxChars), createNativeFunction(context, this, &AsTextField::TextField_set_maxChars));
	prototype->addProperty("maxhscroll", createNativeFunction(context, this, &AsTextField::TextField_get_maxhscroll), 0);
	prototype->addProperty("maxscroll", createNativeFunction(context, this, &AsTextField::TextField_get_maxscroll), 0);
	//prototype->addProperty("menu", createNativeFunction(context, this, &AsTextField::TextField_get_menu), createNativeFunction(context, this, &AsTextField::TextField_set_menu));
	prototype->addProperty("mouseWheelEnabled", createNativeFunction(context, this, &AsTextField::TextField_get_mouseWheelEnabled), createNativeFunction(context, this, &AsTextField::TextField_set_mouseWheelEnabled));
	prototype->addProperty("multiline", createNativeFunction(context, this, &AsTextField::TextField_get_multiline), createNativeFunction(context, this, &AsTextField::TextField_set_multiline));
	prototype->addProperty("_name", createNativeFunction(context, this, &AsTextField::TextField_get_name), createNativeFunction(context, this, &AsTextField::TextField_set_name));
	prototype->addProperty("_parent", createNativeFunction(context, this, &AsTextField::TextField_get_parent), createNativeFunction(context, this, &AsTextField::TextField_set_parent));
	prototype->addProperty("password", createNativeFunction(context, this, &AsTextField::TextField_get_password), createNativeFunction(context, this, &AsTextField::TextField_set_password));
	prototype->addProperty("_quality", createNativeFunction(context, this, &AsTextField::TextField_get_quality), createNativeFunction(context, this, &AsTextField::TextField_set_quality));
	prototype->addProperty("restrict", createNativeFunction(context, this, &AsTextField::TextField_get_restrict), createNativeFunction(context, this, &AsTextField::TextField_set_restrict));
	prototype->addProperty("_rotation", createNativeFunction(context, this, &AsTextField::TextField_get_rotation), createNativeFunction(context, this, &AsTextField::TextField_set_rotation));
	prototype->addProperty("scroll", createNativeFunction(context, this, &AsTextField::TextField_get_scroll), createNativeFunction(context, this, &AsTextField::TextField_set_scroll));
	prototype->addProperty("selectable", createNativeFunction(context, this, &AsTextField::TextField_get_selectable), createNativeFunction(context, this, &AsTextField::TextField_set_selectable));
	prototype->addProperty("sharpness", createNativeFunction(context, this, &AsTextField::TextField_get_sharpness), createNativeFunction(context, this, &AsTextField::TextField_set_sharpness));
	prototype->addProperty("_soundbuftime", createNativeFunction(context, this, &AsTextField::TextField_get_soundbuftime), createNativeFunction(context, this, &AsTextField::TextField_set_soundbuftime));
	//prototype->addProperty("styleSheet", createNativeFunction(context, this, &AsTextField::TextField_get_styleSheet), createNativeFunction(context, this, &AsTextField::TextField_set_styleSheet));
	prototype->addProperty("tabEnabled", createNativeFunction(context, this, &AsTextField::TextField_get_tabEnabled), createNativeFunction(context, this, &AsTextField::TextField_set_tabEnabled));
	prototype->addProperty("tabIndex", createNativeFunction(context, this, &AsTextField::TextField_get_tabIndex), createNativeFunction(context, this, &AsTextField::TextField_set_tabIndex));
	prototype->addProperty("_target", createNativeFunction(context, this, &AsTextField::TextField_get_target), 0);
	prototype->addProperty("text", createNativeFunction(context, this, &AsTextField::TextField_get_text), createNativeFunction(context, this, &AsTextField::TextField_set_text));
	prototype->addProperty("textColor", createNativeFunction(context, this, &AsTextField::TextField_get_textColor), createNativeFunction(context, this, &AsTextField::TextField_set_textColor));
	prototype->addProperty("textWidth", createNativeFunction(context, this, &AsTextField::TextField_get_textWidth), 0);
	prototype->addProperty("textHeight", createNativeFunction(context, this, &AsTextField::TextField_get_textHeight), 0);
	prototype->addProperty("thickness", createNativeFunction(context, this, &AsTextField::TextField_get_thickness), createNativeFunction(context, this, &AsTextField::TextField_set_thickness));
	prototype->addProperty("type", createNativeFunction(context, this, &AsTextField::TextField_get_type), createNativeFunction(context, this, &AsTextField::TextField_set_type));
	prototype->addProperty("_url", createNativeFunction(context, this, &AsTextField::TextField_get_url), 0);
	prototype->addProperty("variable", createNativeFunction(context, this, &AsTextField::TextField_get_variable), createNativeFunction(context, this, &AsTextField::TextField_set_variable));
	prototype->addProperty("_visible", createNativeFunction(context, this, &AsTextField::TextField_get_visible), createNativeFunction(context, this, &AsTextField::TextField_set_visible));
	prototype->addProperty("_width", createNativeFunction(context, this, &AsTextField::TextField_get_width), createNativeFunction(context, this, &AsTextField::TextField_set_width));
	prototype->addProperty("wordWrap", createNativeFunction(context, this, &AsTextField::TextField_get_wordWrap), createNativeFunction(context, this, &AsTextField::TextField_set_wordWrap));
	prototype->addProperty("_x", createNativeFunction(context, this, &AsTextField::TextField_get_x), createNativeFunction(context, this, &AsTextField::TextField_set_x));
	prototype->addProperty("_xmouse", createNativeFunction(context, this, &AsTextField::TextField_get_xmouse), 0);
	prototype->addProperty("_xscale", createNativeFunction(context, this, &AsTextField::TextField_get_xscale), createNativeFunction(context, this, &AsTextField::TextField_set_xscale));
	prototype->addProperty("_y", createNativeFunction(context, this, &AsTextField::TextField_get_y), createNativeFunction(context, this, &AsTextField::TextField_set_y));
	prototype->addProperty("_ymouse", createNativeFunction(context, this, &AsTextField::TextField_get_ymouse), 0);
	prototype->addProperty("_yscale", createNativeFunction(context, this, &AsTextField::TextField_get_yscale), createNativeFunction(context, this, &AsTextField::TextField_set_yscale));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsTextField::initialize(ActionObject* self)
{
}

void AsTextField::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsTextField::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsTextField::TextField_addListener(FlashEditInstance* editInstance, ActionObject* listener) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::addListener not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_getDepth(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::getDepth not implemented" << Endl;
	)
	return 0;
}

Ref< Array > AsTextField::TextField_getFontList(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::getFontList not implemented" << Endl;
	)
	return 0;
}

Ref< FlashTextFormat > AsTextField::TextField_getNewTextFormat(FlashEditInstance* editInstance) const
{
	return editInstance->getTextFormat();
}

Ref< FlashTextFormat > AsTextField::TextField_getTextFormat_0(FlashEditInstance* editInstance) const
{
	return editInstance->getTextFormat();
}

Ref< FlashTextFormat > AsTextField::TextField_getTextFormat_2(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex) const
{
	return editInstance->getTextFormat(beginIndex, endIndex);
}

bool AsTextField::TextField_removeListener(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::removeListener not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_removeTextField(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::removeTextField not implemented" << Endl;
	)
}

void AsTextField::TextField_replaceSel(FlashEditInstance* editInstance, const std::wstring& newText) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::replaceSel not implemented" << Endl;
	)
}

void AsTextField::TextField_replaceText(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex, const std::wstring& newText) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::replaceText not implemented" << Endl;
	)
}

void AsTextField::TextField_setNewTextFormat(FlashEditInstance* editInstance, FlashTextFormat* textFormat) const
{
	editInstance->setTextFormat(textFormat);
}

void AsTextField::TextField_setTextFormat_0(FlashEditInstance* editInstance, FlashTextFormat* textFormat) const
{
	editInstance->setTextFormat(textFormat);
}

void AsTextField::TextField_setTextFormat_2(FlashEditInstance* editInstance, int32_t beginIndex, int32_t endIndex, FlashTextFormat* textFormat) const
{
	editInstance->setTextFormat(textFormat, beginIndex, endIndex);
}

avm_number_t AsTextField::TextField_get_alpha(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_alpha not implemented" << Endl;
	)
	return 100.0f;
}

void AsTextField::TextField_set_alpha(FlashEditInstance* editInstance, avm_number_t alpha) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_alpha not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_antiAliasType(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_antiAliasType not implemented" << Endl;
	)
	return L"normal";
}

void AsTextField::TextField_set_antiAliasType(FlashEditInstance* editInstance, const std::wstring& antiAliasType) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_antiAliasType not implemented" << Endl;
	)
}

Ref< ActionObject > AsTextField::TextField_get_autoSize(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_autoSize not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_autoSize(FlashEditInstance* editInstance, ActionObject* autoSize) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_autoSize not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_background(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_background not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_background(FlashEditInstance* editInstance, bool hasBackground) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_background not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_backgroundColor(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_backgroundColor not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_backgroundColor(FlashEditInstance* editInstance, avm_number_t backgroundColor) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_backgroundColor not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_border(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_border not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_border(FlashEditInstance* editInstance, bool hasBorder) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_border not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_borderColor(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_borderColor not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_borderColor(FlashEditInstance* editInstance, avm_number_t borderColor) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_borderColor not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_bottomScroll(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_bottomScroll not implemented" << Endl;
	)
	return 0;
}

bool AsTextField::TextField_get_condenseWhite(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_condenseWhite not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_condenseWhite(FlashEditInstance* editInstance, bool condenseWhite) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_condenseWhite not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_embedFonts(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_embedFonts not implemented" << Endl;
	)
	return true;
}

void AsTextField::TextField_set_embedFonts(FlashEditInstance* editInstance, bool embedFonts) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_embedFonts not implemented" << Endl;
	)
}

Ref< Array > AsTextField::TextField_get_filters(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_filters not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_filters(FlashEditInstance* editInstance, Array* filters) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_filters not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_gridFitType(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_gridFitType not implemented" << Endl;
	)
	return L"none";
}

void AsTextField::TextField_set_gridFitType(FlashEditInstance* editInstance, const std::wstring& gridFitType) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_gridFitType not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_height(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_height not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_height(FlashEditInstance* editInstance, avm_number_t height) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_height not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_highquality(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_highquality not implemented" << Endl;
	)
	return 2;
}

void AsTextField::TextField_set_highquality(FlashEditInstance* editInstance, avm_number_t highQuality) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_highquality not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_hscroll(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_hscroll not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_hscroll(FlashEditInstance* editInstance, avm_number_t hscroll) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_hscroll not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_html(FlashEditInstance* editInstance) const
{
	return editInstance->getEdit()->renderHtml();
}

void AsTextField::TextField_set_html(FlashEditInstance* editInstance, bool html) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_html not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_htmlText(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_htmlText not implemented" << Endl;
	)
	return L"";
}

void AsTextField::TextField_set_htmlText(FlashEditInstance* editInstance, const std::wstring& htmlText) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_htmlText not implemented" << Endl;
	)
}

int32_t AsTextField::TextField_get_length(FlashEditInstance* editInstance) const
{
	return int32_t(editInstance->getConcatedText().length());
}

int32_t AsTextField::TextField_get_maxChars(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_maxChars not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_maxChars(FlashEditInstance* editInstance, int32_t maxChars) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_maxChars not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_maxhscroll(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_maxhscroll not implemented" << Endl;
	)
	return 0;
}

avm_number_t AsTextField::TextField_get_maxscroll(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_maxscroll not implemented" << Endl;
	)
	return 0;
}

Ref< FlashContextMenu > AsTextField::TextField_get_menu(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_menu not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_menu(FlashEditInstance* editInstance, FlashContextMenu* contextMenu) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_menu not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_mouseWheelEnabled(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_mouseWheelEnabled not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_mouseWheelEnabled(FlashEditInstance* editInstance, bool mouseWheelEnabled) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_mouseWheelEnabled not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_multiline(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_multiline not implemented" << Endl;
	)
	return true;
}

void AsTextField::TextField_set_multiline(FlashEditInstance* editInstance, bool multiliine) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_multiline not implemented" << Endl;
	)
}

std::string AsTextField::TextField_get_name(FlashEditInstance* editInstance) const
{
	return editInstance->getName();
}

void AsTextField::TextField_set_name(FlashEditInstance* editInstance, const std::string& name) const
{
	editInstance->setName(name);
}

Ref< FlashCharacterInstance > AsTextField::TextField_get_parent(FlashEditInstance* editInstance) const
{
	return editInstance->getParent();
}

void AsTextField::TextField_set_parent(FlashEditInstance* editInstance, FlashSpriteInstance* movieClip) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_parent not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_password(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_password not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_password(FlashEditInstance* editInstance, bool password) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_password not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_quality(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_quality not implemented" << Endl;
	)
	return L"BEST";
}

void AsTextField::TextField_set_quality(FlashEditInstance* editInstance, const std::wstring& quality) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_quality not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_restrict(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_restrict not implemented" << Endl;
	)
	return L"";
}

void AsTextField::TextField_set_restrict(FlashEditInstance* editInstance, const std::wstring& restrict) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_restrict not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_rotation(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_rotation not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_rotation(FlashEditInstance* editInstance, avm_number_t rotation) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_rotation not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_scroll(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_scroll not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_scroll(FlashEditInstance* editInstance, avm_number_t scroll) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_scroll not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_selectable(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_selectable not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_selectable(FlashEditInstance* editInstance, bool selectable) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_selectable not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_sharpness(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_sharpness not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_sharpness(FlashEditInstance* editInstance, avm_number_t sharpness) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_sharpness not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_soundbuftime(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_soundbuftime not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_soundbuftime(FlashEditInstance* editInstance, avm_number_t soundbuftime) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_soundbuftime not implemented" << Endl;
	)
}

Ref< FlashStyleSheet > AsTextField::TextField_get_styleSheet(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_styleSheet not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_styleSheet(FlashEditInstance* editInstance, FlashStyleSheet* styleSheet) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_styleSheet not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_tabEnabled(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_tabEnabled not implemented" << Endl;
	)
	return false;
}

void AsTextField::TextField_set_tabEnabled(FlashEditInstance* editInstance, bool tabEnabled) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_tabEnabled not implemented" << Endl;
	)
}

int32_t AsTextField::TextField_get_tabIndex(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_tabIndex not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_tabIndex(FlashEditInstance* editInstance, int32_t tabIndex) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_tabIndex not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_target(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_target not implemented" << Endl;
	)
	return L"";
}

std::wstring AsTextField::TextField_get_text(FlashEditInstance* editInstance) const
{
	return editInstance->getConcatedText();
}

void AsTextField::TextField_set_text(FlashEditInstance* editInstance, const std::wstring& text) const
{
	editInstance->parseText(text);
}

avm_number_t AsTextField::TextField_get_textColor(FlashEditInstance* editInstance) const
{
	const SwfColor& textColor = editInstance->getTextColor();
	return avm_number_t((textColor.red << 16) | (textColor.green << 8) | (textColor.blue));
}

void AsTextField::TextField_set_textColor(FlashEditInstance* editInstance, avm_number_t textColor) const
{
	uint32_t n = uint32_t(textColor);
	SwfColor tc =
	{
		(n >> 16) & 255,
		(n >> 8) & 255,
		(n) & 255,
		255
	};
	editInstance->setTextColor(tc);
}

avm_number_t AsTextField::TextField_get_textWidth(FlashEditInstance* editInstance) const
{
	float width, height;
	if (editInstance->getTextExtents(width, height))
		return avm_number_t(width);
	else
		return avm_number_t(0);
}

avm_number_t AsTextField::TextField_get_textHeight(FlashEditInstance* editInstance) const
{
	float width, height;
	if (editInstance->getTextExtents(width, height))
		return avm_number_t(height);
	else
		return avm_number_t(0);
}

avm_number_t AsTextField::TextField_get_thickness(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_thickness not implemented" << Endl;
	)
	return 0;
}

void AsTextField::TextField_set_thickness(FlashEditInstance* editInstance, avm_number_t thickness) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_thickness not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_type(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_type not implemented" << Endl;
	)
	return L"input";
}

void AsTextField::TextField_set_type(FlashEditInstance* editInstance, const std::wstring& type) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_type not implemented" << Endl;
	)
}

std::wstring AsTextField::TextField_get_url(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_url not implemented" << Endl;
	)
	return L"";
}

std::string AsTextField::TextField_get_variable(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_variable not implemented" << Endl;
	)
	return "";
}

void AsTextField::TextField_set_variable(FlashEditInstance* editInstance, const std::string& url) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_variable not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_visible(FlashEditInstance* editInstance) const
{
	return editInstance->isVisible();
}

void AsTextField::TextField_set_visible(FlashEditInstance* editInstance, bool visible) const
{
	editInstance->setVisible(visible);
}

avm_number_t AsTextField::TextField_get_width(FlashEditInstance* editInstance) const
{
	SwfRect bounds = editInstance->getBounds();
	return bounds.max.x - bounds.min.x;
}

void AsTextField::TextField_set_width(FlashEditInstance* editInstance, avm_number_t width) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_width not implemented" << Endl;
	)
}

bool AsTextField::TextField_get_wordWrap(FlashEditInstance* editInstance) const
{
	return editInstance->getEdit()->wordWrap();
}

void AsTextField::TextField_set_wordWrap(FlashEditInstance* editInstance, bool wordWrap) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_wordWrap not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_x(FlashEditInstance* editInstance) const
{
	Vector2 T, S; float R;
	decomposeTransform(editInstance->getTransform(), T, S, R);
	return T.x / 20.0f;
}

void AsTextField::TextField_set_x(FlashEditInstance* editInstance, avm_number_t x) const
{
	Vector2 T, S; float R;
	decomposeTransform(editInstance->getTransform(), T, S, R);
	T.x = x * 20.0f;
	editInstance->setTransform(composeTransform(T, S, R));
}

avm_number_t AsTextField::TextField_get_xmouse(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_xmouse not implemented" << Endl;
	)
	return 0;
}

avm_number_t AsTextField::TextField_get_xscale(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_xscale not implemented" << Endl;
	)
	return 1;
}

void AsTextField::TextField_set_xscale(FlashEditInstance* editInstance, avm_number_t xscale) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_xscale not implemented" << Endl;
	)
}

avm_number_t AsTextField::TextField_get_y(FlashEditInstance* editInstance) const
{
	Vector2 T, S; float R;
	decomposeTransform(editInstance->getTransform(), T, S, R);
	return T.y / 20.0f;
}

void AsTextField::TextField_set_y(FlashEditInstance* editInstance, avm_number_t y) const
{
	Vector2 T, S; float R;
	decomposeTransform(editInstance->getTransform(), T, S, R);
	T.y = y * 20.0f;
}

avm_number_t AsTextField::TextField_get_ymouse(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_ymouse not implemented" << Endl;
	)
	return 0;
}

avm_number_t AsTextField::TextField_get_yscale(FlashEditInstance* editInstance) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::get_yscale not implemented" << Endl;
	)
	return 1;
}

void AsTextField::TextField_set_yscale(FlashEditInstance* editInstance, avm_number_t yscale) const
{
	T_IF_VERBOSE(
		log::warning << L"TextField::set_yscale not implemented" << Endl;
	)
}

	}
}
