#pragma once

#include <set>
#include "Core/Ref.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{
    namespace ui
    {

class Edit;
class EditList;
class EditListEditEvent;
class RichEdit;

    }

    namespace store
    {

class PublishWizardDialog : public ui::ConfigDialog
{
    T_RTTI_CLASS;

public:
    PublishWizardDialog();

    bool create(ui::Widget* parent, const std::wstring& instanceName);

    std::wstring getName() const;

    std::wstring getAuthor() const;

    std::wstring getEMail() const;

    std::wstring getPhone() const;

    std::wstring getSite() const;

    std::wstring getDescription() const;

    std::set< std::wstring > getTags() const;

private:
    Ref< ui::Edit > m_editName;
    Ref< ui::Edit > m_editAuthor;
    Ref< ui::Edit > m_editEMail;
    Ref< ui::Edit > m_editPhone;
    Ref< ui::Edit > m_editSite;
    Ref< ui::RichEdit > m_editDescription;
    Ref< ui::EditList > m_editListTags;

    void eventTagEdit(ui::EditListEditEvent* event);
};

    }
}