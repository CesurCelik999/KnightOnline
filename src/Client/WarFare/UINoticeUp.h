#pragma once

#include <N3Base/N3UIBase.h>

#include <string>
#include <utility>
#include <vector>

class CN3UIString;

// In-game top-right notices. Does not use HTML or require .uif asset changes.
class CUINoticeUp : public CN3UIBase
{
public:
    using Entry = std::pair<std::string, std::string>; // title, explanation
    static constexpr size_t MaxEntries = 8;

    void Init(CN3UIBase* parent) override;
    void Render() override;
    void Release() override;
    bool ReceiveMessage(CN3UIBase* sender, uint32_t message) override;
    void SetEntries(const std::vector<Entry>& entries);
    void PositionAtTopRight(int viewportWidth);

private:
    void UpdateHeight();

    std::vector<CN3UIString*> m_rows;
    std::vector<Entry> m_entries;
    CN3UIString* m_heading = nullptr;
    CN3UIString* m_description = nullptr;
    int m_selected = -1;
};
