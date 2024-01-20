#include "pch.h"
#include "CBlueprintEditor2.h"
#include "CEngine.h"

static ImEx::MostRecentlyUsedList Application_GetMostRecentlyOpenFileList()
{
    return ImEx::MostRecentlyUsedList("MostRecentlyOpenList");
}

static EntryPointNode* FindEntryPointNode(Blueprint& blueprint)
{
    for (auto& node : blueprint.GetNodes())
    {
        if (node->GetTypeInfo().m_Id == EntryPointNode::GetStaticTypeInfo().m_Id)
        {
            return static_cast<EntryPointNode*>(node);
        }
    }

    return nullptr;
}

Action::Action(string_view name, OnTriggeredEvent::Delegate delegate)
    : m_Name(to_string(name))
{
    if (delegate)
        OnTriggered += std::move(delegate);
}

void Action::SetName(string_view name)
{
    if (m_Name == name)
        return;

    m_Name = to_string(name);

    OnChange(this);
}

const crude_blueprint::string& Action::GetName() const
{
    return m_Name;
}

void Action::SetEnabled(bool set)
{
    if (m_IsEnabled == set)
        return;

    m_IsEnabled = set;

    OnChange(this);
}

bool Action::IsEnabled() const
{
    return m_IsEnabled;
}

void Action::Execute()
{
    LOGV("Action: %s", m_Name.c_str());
    OnTriggered();
}

CBlueprintEditor2::CBlueprintEditor2()
    : CEditor(EDITOR_TYPE::BLUEPRINT)
    , m_Editor(nullptr)
    , m_Blueprint(nullptr)
{
    m_File_New = {"New", [this] { File_New(); }};
    m_File_Open = {"Open...", [this] { File_Open(); }};
    m_File_SaveAs = {"Save As...", [this] { File_SaveAs(); }};
    m_File_Save = {"Save", [this] { File_Save(); }};
    m_File_Close = {"Close", [this] { File_Close(); }};
    m_File_Exit = {"Exit", [this] { File_Exit(); }};

    m_Edit_Undo = {"Undo", [this] { Edit_Undo(); }};
    m_Edit_Redo = {"Redo", [this] { Edit_Redo(); }};
    m_Edit_Cut = {"Cut", [this] { Edit_Cut(); }};
    m_Edit_Copy = {"Copy", [this] { Edit_Copy(); }};
    m_Edit_Paste = {"Paste", [this] { Edit_Paste(); }};
    m_Edit_Duplicate = {"Duplicate", [this] { Edit_Duplicate(); }};
    m_Edit_Delete = {"Delete", [this] { Edit_Delete(); }};
    m_Edit_SelectAll = {"Select All", [this] { Edit_SelectAll(); }};

    m_View_NavigateBackward = {"Navigate Backward", [this] { View_NavigateBackward(); }};
    m_View_NavigateForward = {"Navigate Forward", [this] { View_NavigateForward(); }};

    m_Blueprint_Start = {"Start", [this] { Blueprint_Start(); }};
    m_Blueprint_Step = {"Step", [this] { Blueprint_Step(); }};
    m_Blueprint_Stop = {"Stop", [this] { Blueprint_Stop(); }};
    m_Blueprint_Run = {"Run", [this] { Blueprint_Run(); }};
}

CBlueprintEditor2::~CBlueprintEditor2()
{
    ed::DestroyEditor(m_Editor);
}

void CBlueprintEditor2::init()
{
    crude_logger::OverlayLogger::SetCurrent(&m_OverlayLogger);

    m_OverlayLogger.AddKeyword("Node");
    m_OverlayLogger.AddKeyword("Pin");
    m_OverlayLogger.AddKeyword("Link");
    m_OverlayLogger.AddKeyword("CreateNodeDialog");
    m_OverlayLogger.AddKeyword("NodeContextMenu");
    m_OverlayLogger.AddKeyword("PinContextMenu");
    m_OverlayLogger.AddKeyword("LinkContextMenu");

    ImEx::MostRecentlyUsedList::Install(ImGui::GetCurrentContext());

    PrintNode::s_PrintFunction = [](const PrintNode& node, string_view message) {
        LOGI("PrintNode(%" PRIu32 "): \"%" PRI_sv "\"", node.m_Id, FMT_sv(message));
    };

    ed::Config config;
    InstallDocumentCallbacks(config);
    m_Editor = ed::CreateEditor(&config);
    ed::SetCurrentEditor(m_Editor);

    CreateExampleDocument();

    for (auto nodeTypeInfo : m_Blueprint->GetNodeRegistry()->GetTypes())
        m_OverlayLogger.AddKeyword(nodeTypeInfo->m_Name);

    ed::SetCurrentEditor(nullptr);
}

void CBlueprintEditor2::render(bool* open)
{
    if (!ImGui::Begin("Blueprint Editor2", open))
    {
        *open = false;
        ImGui::End();
        return;
    }

    render();

    ImGui::End();
}

void CBlueprintEditor2::render()
{
    ed::SetCurrentEditor(m_Editor);

    UpdateActions();

    ShowMainMenu();
    ShowToolbar();

    ImGui::Separator();

    if (m_Blueprint)
    {
        DebugOverlay debugOverlay(*m_Blueprint);

        ed::Begin("###main_editor");

        CommitBlueprintNodes(*m_Blueprint, debugOverlay);

        HandleCreateAction(*m_Document);
        HandleDestroyAction(*m_Document);
        HandleContextMenuAction(*m_Blueprint);

        ShowDialogs(*m_Document);

        ShowInfoTooltip(*m_Blueprint);

        ed::End();
    }
    else
    {
        ImGui::Dummy(ImGui::GetContentRegionAvail());
    }

    m_OverlayLogger.Update(ImGui::GetIO().DeltaTime);
    m_OverlayLogger.Draw(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    ed::SetCurrentEditor(nullptr);

    // ImGui::ShowMetricsWindow();
}

void CBlueprintEditor2::InstallDocumentCallbacks(ed::Config& config)
{
    config.UserPointer = this;
    // config.BeginSaveSession = [](void* userPointer)
    //{
    //     auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);

    //    if (self->m_Document)
    //        self->m_Document->OnSaveBegin();
    //};
    // config.EndSaveSession = [](void* userPointer)
    //{
    //    auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);

    //    if (self->m_Document)
    //        self->m_Document->OnSaveEnd();
    //};
    // config.SaveSettingsJson = [](const crude_json::value& state, ed::SaveReasonFlags reason, void* userPointer)
    // -> bool
    //{
    //    auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);

    //    if (self->m_Document)
    //        return self->m_Document->OnSaveState(state, reason);
    //    else
    //        return false;
    //};
    // config.LoadSettingsJson = [](void* userPointer) -> crude_json::value
    //{
    //    auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);

    //    if (self->m_Document)
    //        return self->m_Document->OnLoadState();
    //    else
    //        return {};
    //};
    // config.SaveNodeSettingsJson = [](ed::NodeId nodeId, const crude_json::value& value, ed::SaveReasonFlags
    // reason, void* userPointer) -> bool
    //{
    //    auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);

    //    if (self->m_Document)
    //        return self->m_Document->OnSaveNodeState(static_cast<uint32_t>(nodeId.Get()), value, reason);
    //    else
    //        return false;
    //};
    // config.LoadNodeSettingsJson = [](ed::NodeId nodeId, void* userPointer) -> crude_json::value
    //{
    //    auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);

    //    if (self->m_Document)
    //        return self->m_Document->OnLoadNodeState(static_cast<uint32_t>(nodeId.Get()));
    //    else
    //        return {};
    //};

    struct TransactionWrapper final : ed::ITransaction
    {
        static const char* ActionToString(ed::TransactionAction action)
        {
            switch (action)
            {
            case ed::TransactionAction::Unknown:
                return "Unknown";
            case ed::TransactionAction::Navigation:
                return "Navigation";
            case ed::TransactionAction::Drag:
                return "Drag";
            }

            return "";
        }

        TransactionWrapper(shared_ptr<Document::UndoTransaction> transaction)
            : m_Transaction(std::move(transaction))
        {
        }

        void AddAction(ed::TransactionAction action, const char* name) override
        {
            ++m_ActionCount;

            m_Transaction->AddAction(name);
        }

        void Commit() override
        {
            ImGuiTextBuffer name;
            if (m_ActionCount == m_DragActionCount && m_DragActionCount > 1)
                name.appendf("Drag %d nodes", static_cast<int>(m_DragActionCount));

            m_Transaction->Commit(name.c_str());
        }

        void Discard() override { m_Transaction->Discard(); }

        void AddAction(ed::TransactionAction action, ed::NodeId nodeId, const char* name) override
        {
            ++m_ActionCount;

            m_NodeIds.push_back(nodeId);

            if (action == ed::TransactionAction::Drag)
            {
                ++m_DragActionCount;

                auto& blueprint = m_Transaction->GetDocument()->GetBlueprint();
                auto node = blueprint.FindNode(static_cast<uint32_t>(nodeId.Get()));

                m_Transaction->AddAction("Drag %" PRI_node, FMT_node(node));
            }
            else
                m_Transaction->AddAction(name);
        }

        shared_ptr<Document::UndoTransaction> m_Transaction;
        size_t m_ActionCount = 0;
        size_t m_DragActionCount = 0;
        vector<ed::NodeId> m_NodeIds;
    };

    config.TransactionInterface.Constructor = [](const char* name, void* userPointer) -> ed::ITransaction* {
        auto self = reinterpret_cast<CBlueprintEditor2*>(userPointer);
        return new TransactionWrapper(self->m_Document->BeginUndoTransaction(name));
    };

    config.TransactionInterface.Destructor = [](ed::ITransaction* transaction, void* userPointer) {
        delete transaction;
    };

    config.TransactionInterface.UserPointer = this;
}

void CBlueprintEditor2::CreateExampleDocument()
{
    using namespace crude_blueprint;

    m_Document = make_unique<Document>();
    m_Blueprint = &m_Document->m_Blueprint;

    auto printNode2Node = m_Blueprint->CreateNode<PrintNode>();
    ed::SetNodePosition(printNode2Node->m_Id, ImVec2(828, 111));
    auto entryPointNode = m_Blueprint->CreateNode<EntryPointNode>();
    ed::SetNodePosition(entryPointNode->m_Id, ImVec2(-20, 95));
    auto printNode1Node = m_Blueprint->CreateNode<PrintNode>();
    ed::SetNodePosition(printNode1Node->m_Id, ImVec2(828, -1));
    auto flipFlopNode = m_Blueprint->CreateNode<FlipFlopNode>();
    ed::SetNodePosition(flipFlopNode->m_Id, ImVec2(408, -1));
    auto toStringNode = m_Blueprint->CreateNode<ToStringNode>();
    ed::SetNodePosition(toStringNode->m_Id, ImVec2(617, 111));
    auto doNNode = m_Blueprint->CreateNode<DoNNode>();
    ed::SetNodePosition(doNNode->m_Id, ImVec2(168, 95));
    auto addNode = m_Blueprint->CreateNode<AddNode>();
    ed::SetNodePosition(addNode->m_Id, ImVec2(404, 159));

    entryPointNode->m_Exit.LinkTo(doNNode->m_Enter);

    doNNode->m_N.SetValue(5);
    doNNode->m_Exit.LinkTo(flipFlopNode->m_Enter);

    toStringNode->m_Value.LinkTo(addNode->m_Result);

    addNode->m_A.LinkTo(doNNode->m_Counter);
    addNode->m_B.SetValue(3);

    toStringNode->m_Exit.LinkTo(printNode2Node->m_Enter);

    printNode1Node->m_String.SetValue("FlipFlop slot A!");
    printNode2Node->m_String.SetValue("FlipFlop slot B!");
    printNode2Node->m_String.LinkTo(toStringNode->m_String);

    flipFlopNode->m_A.LinkTo(printNode1Node->m_Enter);
    flipFlopNode->m_B.LinkTo(toStringNode->m_Enter);
}

void CBlueprintEditor2::UpdateTitle()
{
    wstring title = GetName();

    if (File_IsOpen())
    {
        title += L" - ";
        title += ToWstring(m_Document->m_Name.c_str());

        if (File_IsModified())
            title += L"*";
    }

    SetWindowText(CEngine::GetInst()->GetMainWind(), title.c_str());
}

void CBlueprintEditor2::UpdateActions()
{
    auto hasDocument = File_IsOpen();
    auto hasUndo = hasDocument && !m_Document->m_Undo.empty();
    auto hasRedo = hasDocument && !m_Document->m_Redo.empty();
    // auto isModified  = hasDocument && File_IsModified();

    m_File_Close.SetEnabled(hasDocument);
    m_File_SaveAs.SetEnabled(hasDocument);
    m_File_Save.SetEnabled(hasDocument);

    m_Edit_Undo.SetEnabled(hasUndo);
    m_Edit_Redo.SetEnabled(hasRedo);
    m_Edit_Cut.SetEnabled(hasDocument && false);
    m_Edit_Copy.SetEnabled(hasDocument && false);
    m_Edit_Paste.SetEnabled(hasDocument && false);
    m_Edit_Duplicate.SetEnabled(hasDocument && false);
    m_Edit_Delete.SetEnabled(hasDocument && false);
    m_Edit_SelectAll.SetEnabled(hasDocument && false);

    m_View_NavigateBackward.SetEnabled(hasDocument && false);
    m_View_NavigateForward.SetEnabled(hasDocument && false);

    auto entryNode = m_Blueprint ? FindEntryPointNode(*m_Blueprint) : nullptr;

    bool hasBlueprint = (m_Blueprint != nullptr);
    bool hasEntryPoint = (entryNode != nullptr);
    bool isExecuting = hasBlueprint && (m_Blueprint->CurrentNode() != nullptr);

    m_Blueprint_Start.SetEnabled(hasBlueprint && hasEntryPoint);
    m_Blueprint_Step.SetEnabled(hasBlueprint && isExecuting);
    m_Blueprint_Stop.SetEnabled(hasBlueprint && isExecuting);
    m_Blueprint_Run.SetEnabled(hasBlueprint && hasEntryPoint);
}

void CBlueprintEditor2::ShowMainMenu()
{
    auto menuAction = [](Action& action) {
        if (ImGui::MenuItem(action.GetName().c_str(), nullptr, nullptr, action.IsEnabled()))
        {
            action.Execute();
        }
    };

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            auto mostRecentlyOpenFiles = Application_GetMostRecentlyOpenFileList();

            menuAction(m_File_New);
            ImGui::Separator();
            menuAction(m_File_Open);
            if (ImGui::BeginMenu("Open Recent", !mostRecentlyOpenFiles.GetList().empty()))
            {
                for (auto& entry : mostRecentlyOpenFiles.GetList())
                    if (ImGui::MenuItem(entry.c_str()))
                        File_Open(entry.c_str());
                ImGui::Separator();
                if (ImGui::MenuItem("Clear Recently Opened"))
                    mostRecentlyOpenFiles.Clear();
                ImGui::EndMenu();
            }
            ImGui::Separator();
            menuAction(m_File_SaveAs);
            menuAction(m_File_Save);
            ImGui::Separator();
            menuAction(m_File_Close);
            ImGui::Separator();
            menuAction(m_File_Exit);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            menuAction(m_Edit_Undo);
            menuAction(m_Edit_Redo);
            ImGui::Separator();
            menuAction(m_Edit_Cut);
            menuAction(m_Edit_Copy);
            menuAction(m_Edit_Paste);
            menuAction(m_Edit_Duplicate);
            menuAction(m_Edit_Delete);
            ImGui::Separator();
            menuAction(m_Edit_SelectAll);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            menuAction(m_View_NavigateBackward);
            menuAction(m_View_NavigateForward);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Blueprint"))
        {
            menuAction(m_Blueprint_Start);
            menuAction(m_Blueprint_Step);
            menuAction(m_Blueprint_Stop);
            ImGui::Separator();
            menuAction(m_Blueprint_Run);

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void CBlueprintEditor2::ShowToolbar()
{
    auto toolbarAction = [](Action& action) {
        ImEx::ScopedDisableItem disableAction(!action.IsEnabled());

        if (ImGui::Button(action.GetName().c_str()))
        {
            action.Execute();
        }
    };

    toolbarAction(m_File_Open);
    ImGui::SameLine();
    toolbarAction(m_File_Save);
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    toolbarAction(m_Edit_Undo);
    ImGui::SameLine();
    toolbarAction(m_Edit_Redo);
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    toolbarAction(m_Blueprint_Start);
    ImGui::SameLine();
    toolbarAction(m_Blueprint_Step);
    ImGui::SameLine();
    toolbarAction(m_Blueprint_Stop);
    ImGui::SameLine();
    toolbarAction(m_Blueprint_Run);

    ImGui::SameLine();
    ImGui::Text("Status: %s", m_Blueprint ? StepResultToString(m_Blueprint->LastStepResult()) : "-");

    if (auto currentNode = m_Blueprint ? m_Blueprint->CurrentNode() : nullptr)
    {
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Text("Current: %" PRI_node, FMT_node(currentNode));

        auto nextNode = m_Blueprint->NextNode();
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        if (nextNode)
            ImGui::Text("Next: %" PRI_node, FMT_node(nextNode));
        else
            ImGui::Text("Next: -");
    }
}

void CBlueprintEditor2::CommitBlueprintNodes(Blueprint& blueprint, DebugOverlay& debugOverlay)
{
    const auto iconSize = ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight());

    debugOverlay.Begin();

    // Commit all nodes to editor
    for (auto& node : blueprint.GetNodes())
    {
        ed::BeginNode(node->m_Id);

        // General node layout:
        //
        // +-----------------------------------+
        // | Title                             |
        // | +-----------[ Dummy ]-----------+ |
        // | +---------------+   +-----------+ |
        // | | o Pin         |   |   Out B o | |
        // | | o Pin <Value> |   |   Out A o | |
        // | | o Pin         |   |           | |
        // | +---------------+   +-----------+ |
        // +-----------------------------------+

        // Show title if node has one.
        auto nodeName = node->GetName();
        if (!nodeName.empty())
        {
            auto headerBackgroundRenderer = ImEx::ItemBackgroundRenderer([node](ImDrawList* drawList) {
                auto border = ed::GetStyle().NodeBorderWidth;
                auto rounding = ed::GetStyle().NodeRounding;

                auto itemMin = ImGui::GetItemRectMin();
                auto itemMax = ImGui::GetItemRectMax();

                auto nodeStart = ed::GetNodePosition(node->m_Id);
                auto nodeSize = ed::GetNodeSize(node->m_Id);

                itemMin = nodeStart;
                itemMin.x = itemMin.x + border - 0.5f;
                itemMin.y = itemMin.y + border - 0.5f;
                itemMax.x = nodeStart.x + nodeSize.x - border + 0.5f;
                itemMax.y = itemMax.y + ImGui::GetStyle().ItemSpacing.y + 0.5f;

                drawList->AddRectFilled(itemMin, itemMax, IM_COL32(255, 255, 255, 64), rounding,
                                        ImDrawFlags_RoundCornersTop);

                // drawList->AddRectFilledMultiColor(itemMin, itemMax, IM_COL32(255, 0, 0, 64), rounding,
                // ImDrawFlags_RoundCornersTop);
            });

            // ImGui::PushFont(HeaderFont());
            // ImGui::TextUnformatted(nodeName.data(), nodeName.data() + nodeName.size());
            // ImGui::PopFont();

            ImGui::TextUnformatted(nodeName.data(), nodeName.data() + nodeName.size());

            // ImEx::Debug_DrawItemRect();
            ImGui::Spacing();
        }

        ImGui::Dummy(ImVec2(100.0f, 0.0f)); // For minimum node width

        crude_layout::Grid layout;
        layout.Begin(node->m_Id, 2, 100.0f);
        layout.SetColumnAlignment(0.0f);

        // Draw column with input pins.
        for (auto& pin : node->GetInputPins())
        {
            // Add a bit of spacing to separate pins and make value not cramped
            ImGui::Spacing();

            // Input pin layout:
            //
            //     +-[1]---+-[2]------+-[3]----------+
            //     |       |          |              |
            //    [X] Icon | Pin Name | Value/Editor |
            //     |       |          |              |
            //     +-------+----------+--------------+

            ed::BeginPin(pin->m_Id, ed::PinKind::Input);
            // [X] - Tell editor to put pivot point in the middle of
            //       the left side of the pin. This is the point
            //       where link will be hooked to.
            //
            //       By default pivot is in pin center point which
            //       does not look good for blueprint nodes.
            ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));

            // [1] - Icon
            Icon(iconSize, PinTypeToIconType(pin->GetType()), blueprint.HasPinAnyLink(*pin),
                 PinTypeToColor(pin->GetValueType()));

            // [2] - Show pin name if it has one
            if (!pin->m_Name.empty())
            {
                ImGui::SameLine();
                ImGui::TextUnformatted(pin->m_Name.data(), pin->m_Name.data() + pin->m_Name.size());
            }

            // [3] - Show value/editor when pin is not linked to anything
            if (!blueprint.HasPinAnyLink(*pin))
            {
                ImGui::SameLine();
                DrawPinValueWithEditor(*pin);
            }

            ed::EndPin();

            // [Debug Overlay] Show value of the pin if node is currently executed
            debugOverlay.DrawInputPin(*pin);

            layout.NextRow();
        }

        layout.SetColumnAlignment(1.0f);
        layout.NextColumn();

        // Draw column with output pins.
        for (auto& pin : node->GetOutputPins())
        {
            // Add a bit of spacing to separate pins and make value not cramped
            ImGui::Spacing();

            // Output pin layout:
            //
            //    +-[1]------+-[2]---+
            //    |          |       |
            //    | Pin Name | Icon [X]
            //    |          |       |
            //    +----------+-------+

            ed::BeginPin(pin->m_Id, ed::PinKind::Output);

            // [X] - Tell editor to put pivot point in the middle of
            //       the right side of the pin. This is the point
            //       where link will be hooked to.
            //
            //       By default pivot is in pin center point which
            //       does not look good for blueprint nodes.
            ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));

            // [1] - Show pin name if it has one
            if (!pin->m_Name.empty())
            {
                ImGui::TextUnformatted(pin->m_Name.data(), pin->m_Name.data() + pin->m_Name.size());
                ImGui::SameLine();
            }

            // [2] - Show icon
            Icon(iconSize, PinTypeToIconType(pin->GetType()), blueprint.HasPinAnyLink(*pin),
                 PinTypeToColor(pin->GetValueType()));

            ed::EndPin();

            // [Debug Overlay] Show value of the pin if node is currently executed
            debugOverlay.DrawOutputPin(*pin);

            layout.NextRow();
        }

        layout.End();

        // ImEx::Debug_DrawItemRect();

        ed::EndNode();

        // ImEx::Debug_DrawItemRect();

        // [Debug Overlay] Show cursor over node
        debugOverlay.DrawNode(*node);
    }

    // Commit all links to editor
    for (auto& pin : blueprint.GetPins())
    {
        if (!pin->m_Link)
            continue;

        // To keep things simple, link id is same as pin id.
        ed::Link(pin->m_Id, pin->m_Id, pin->m_Link->m_Id, PinTypeToColor(pin->GetValueType()));
    }

    debugOverlay.End();
}

void CBlueprintEditor2::HandleCreateAction(Document& document)
{
    Blueprint& blueprint = document.m_Blueprint;

    ItemBuilder itemBuilder;

    if (!itemBuilder)
        return;

    if (auto linkBuilder = itemBuilder.QueryNewLink())
    {
        auto startPin = blueprint.FindPin(static_cast<uint32_t>(linkBuilder->m_StartPinId.Get()));
        auto endPin = blueprint.FindPin(static_cast<uint32_t>(linkBuilder->m_EndPinId.Get()));

        // Editor return pins in order draw by the user. It is up to the
        // user to determine if it is valid. In blueprints we accept only links
        // from receivers to providers. Other graph types may allow bi-directional
        // links between nodes and this ordering make this feature possible.
        if (endPin->IsReceiver() && startPin->IsProvider())
            ImSwap(startPin, endPin);

        if (auto canLinkResult = startPin->CanLinkTo(*endPin))
        {
            ed::Suspend();
            ImGui::BeginTooltip();
            ImGui::Text("Valid Link%s%s", canLinkResult.Reason().empty() ? "" : ": ",
                        canLinkResult.Reason().empty() ? "" : canLinkResult.Reason().c_str());
            ImGui::Separator();
            ImGui::TextUnformatted("From:");
            ImGui::Bullet();
            ImGui::Text("%" PRI_pin, FMT_pin(startPin));
            ImGui::Bullet();
            ImGui::Text("%" PRI_node, FMT_node(startPin->m_Node));
            ImGui::TextUnformatted("To:");
            ImGui::Bullet();
            ImGui::Text("%" PRI_pin, FMT_pin(endPin));
            ImGui::Bullet();
            ImGui::Text("%" PRI_node, FMT_node(endPin->m_Node));
            ImGui::EndTooltip();
            ed::Resume();

            if (linkBuilder->Accept())
            {
                auto transaction = document.BeginUndoTransaction("Create Link");

                if (startPin->LinkTo(*endPin))
                    LOGV("[HandleCreateAction] %" PRI_pin " linked with %" PRI_pin, FMT_pin(startPin), FMT_pin(endPin));
                else
                    transaction->Discard();
            }
        }
        else
        {
            ed::Suspend();
            ImGui::SetTooltip("Invalid Link: %s", canLinkResult.Reason().c_str());
            ed::Resume();

            linkBuilder->Reject();
        }
    }
    else if (auto nodeBuilder = itemBuilder.QueryNewNode())
    {
        // Arguably creation of node is simpler than a link.
        ed::Suspend();
        ImGui::SetTooltip("Create Node...");
        ed::Resume();

        // Node builder accept return true when user release mouse button.
        // When this happen we request CreateNodeDialog to open.
        if (nodeBuilder->Accept())
        {
            // Get node from which link was pulled (if any). After creating
            // node we will try to make link with first matching pin of the node.
            auto pin = blueprint.FindPin(static_cast<uint32_t>(nodeBuilder->m_PinId.Get()));

            ed::Suspend();
            LOGV("[HandleCreateAction] Open CreateNodeDialog");
            m_CreateNodeDailog.Open(pin);
            ed::Resume();
        }
    }
}

void CBlueprintEditor2::HandleDestroyAction(Document& document)
{
    Blueprint& blueprint = document.m_Blueprint;

    ItemDeleter itemDeleter;

    if (!itemDeleter)
        return;

    auto deferredTransaction = document.GetDeferredUndoTransaction("Destroy Action");

    vector<Node*> nodesToDelete;
    uint32_t brokenLinkCount = 0;

    // Process all nodes marked for deletion
    while (auto nodeDeleter = itemDeleter.QueryDeletedNode())
    {
        deferredTransaction->Begin("Delete Item");

        // Remove node, pass 'true' so links attached to node will also be queued for deletion.
        if (nodeDeleter->Accept(true))
        {
            auto node = blueprint.FindNode(static_cast<uint32_t>(nodeDeleter->m_NodeId.Get()));
            if (node != nullptr)
                // Queue nodes for deletion. We need to serve links first to avoid crash.
                nodesToDelete.push_back(node);
        }
    }

    // Process all links marked for deletion
    while (auto linkDeleter = itemDeleter.QueryDeleteLink())
    {
        deferredTransaction->Begin("Delete Item");

        if (linkDeleter->Accept())
        {
            auto startPin = blueprint.FindPin(static_cast<uint32_t>(linkDeleter->m_StartPinId.Get()));
            if (startPin != nullptr && startPin->IsLinked())
            {
                LOGV("[HandleDestroyAction] %" PRI_pin " unlinked from %" PRI_pin, FMT_pin(startPin),
                     FMT_pin(startPin->GetLink()));
                startPin->Unlink();
                ++brokenLinkCount;
            }
        }
    }

    // After links was removed, now it is safe to delete nodes.
    for (auto node : nodesToDelete)
    {
        LOGV("[HandleDestroyAction] %" PRI_node, FMT_node(node));
        blueprint.DeleteNode(node);
    }

    if (!nodesToDelete.empty() || brokenLinkCount)
    {
        LOGV("[HandleDestroyAction] %" PRIu32 " node%s deleted, %" PRIu32 " link%s broken",
             static_cast<uint32_t>(nodesToDelete.size()), nodesToDelete.size() != 1 ? "s" : "", brokenLinkCount,
             brokenLinkCount != 1 ? "s" : "");
    }
}

void CBlueprintEditor2::HandleContextMenuAction(Blueprint& blueprint)
{
    if (ed::ShowBackgroundContextMenu())
    {
        ed::Suspend();
        LOGV("[HandleContextMenuAction] Open CreateNodeDialog");
        m_CreateNodeDailog.Open();
        ed::Resume();
    }

    ed::NodeId contextNodeId;
    if (ed::ShowNodeContextMenu(&contextNodeId))
    {
        auto node = blueprint.FindNode(static_cast<uint32_t>(contextNodeId.Get()));

        ed::Suspend();
        LOGV("[HandleContextMenuAction] Open NodeContextMenu for %" PRI_node, FMT_node(node));
        m_NodeContextMenu.Open(node);
        ed::Resume();
    }

    ed::PinId contextPinId;
    if (ed::ShowPinContextMenu(&contextPinId))
    {
        auto pin = blueprint.FindPin(static_cast<uint32_t>(contextPinId.Get()));

        ed::Suspend();
        LOGV("[HandleContextMenuAction] Open PinContextMenu for %" PRI_pin, FMT_pin(pin));
        m_PinContextMenu.Open(pin);
        ed::Resume();
    }

    ed::LinkId contextLinkId;
    if (ed::ShowLinkContextMenu(&contextLinkId))
    {
        auto pin = blueprint.FindPin(static_cast<uint32_t>(contextLinkId.Get()));

        ed::Suspend();
        LOGV("[HandleContextMenuAction] Open LinkContextMenu for %" PRI_pin, FMT_pin(pin));
        m_LinkContextMenu.Open(pin);
        ed::Resume();
    }
}

void CBlueprintEditor2::ShowDialogs(Document& document)
{
    Blueprint& blueprint = document.m_Blueprint;

    ed::Suspend();

    m_CreateNodeDailog.Show(document);
    m_NodeContextMenu.Show(blueprint);
    m_PinContextMenu.Show(blueprint);
    m_LinkContextMenu.Show(blueprint);

    ed::Resume();
}

void CBlueprintEditor2::ShowInfoTooltip(Blueprint& blueprint)
{
    if (!ed::IsActive())
        return;

    auto hoveredNode = blueprint.FindNode(static_cast<uint32_t>(ed::GetHoveredNode().Get()));
    auto hoveredPin = blueprint.FindPin(static_cast<uint32_t>(ed::GetHoveredPin().Get()));
    if (!hoveredNode && hoveredPin)
        hoveredNode = hoveredPin->m_Node;

    auto pinTooltip = [](const char* label, const Pin& pin, bool showNode) {
        ImGui::TextUnformatted(label);
        ImGui::Bullet();
        ImGui::Text("ID: %" PRIu32, pin.m_Id);
        if (!pin.m_Name.empty())
        {
            ImGui::Bullet();
            ImGui::Text("Name: %" PRI_sv, FMT_sv(pin.m_Name));
        }
        if (showNode && pin.m_Node)
        {
            auto nodeName = pin.m_Node->GetName();
            ImGui::Bullet();
            ImGui::Text("Node: %" PRI_sv, FMT_sv(nodeName));
        }
        ImGui::Bullet();
        ImGui::Text("Type: %s", PinTypeToString(pin.GetType()));
        ImGui::Bullet();
        ImGui::Text("Value Type: %s", PinTypeToString(pin.GetValueType()));

        string flags;
        if (pin.IsLinked())
            flags += "linked, ";
        if (pin.IsInput())
            flags += "input, ";
        if (pin.IsOutput())
            flags += "output, ";
        if (pin.IsProvider())
            flags += "provider, ";
        if (pin.IsReceiver())
            flags += "receiver, ";
        if (!flags.empty())
            flags = flags.substr(0, flags.size() - 2);
        ImGui::Bullet();
        ImGui::Text("Flags: %s", flags.c_str());
    };

    if (hoveredNode)
    {
        auto nodeTypeInfo = hoveredNode->GetTypeInfo();
        auto nodeName = hoveredNode->GetName();

        ed::Suspend();
        ImGui::BeginTooltip();

        ImGui::Text("Node ID: %" PRIu32, hoveredNode->m_Id);
        ImGui::Text("Name: %" PRI_sv, FMT_sv(nodeName));
        ImGui::Separator();
        ImGui::TextUnformatted("Type:");
        ImGui::Bullet();
        ImGui::Text("ID: 0x%08" PRIX32, nodeTypeInfo.m_Id);
        ImGui::Bullet();
        ImGui::Text("Name: %" PRI_sv, FMT_sv(nodeTypeInfo.m_Name));

        if (hoveredPin)
        {
            ImGui::Separator();
            pinTooltip("Pin", *hoveredPin, false);
        }

        ImGui::EndTooltip();
        ed::Resume();
    }
    else if (auto hoveredLinkId = ed::GetHoveredLink())
    {
        ed::PinId startPinId, endPinId;
        ed::GetLinkPins(hoveredLinkId, &startPinId, &endPinId);

        auto startPin = blueprint.FindPin(static_cast<uint32_t>(startPinId.Get()));
        auto endPin = blueprint.FindPin(static_cast<uint32_t>(endPinId.Get()));

        ed::Suspend();
        ImGui::BeginTooltip();

        ImGui::Text("Link ID: %" PRIu32, startPin->m_Id);
        ImGui::Text("Type: %s", PinTypeToString(startPin->GetValueType()));
        ImGui::Separator();
        pinTooltip("Start Pin:", *startPin, true);
        ImGui::Separator();
        pinTooltip("End Pin:", *endPin, true);

        ImGui::EndTooltip();
        ed::Resume();
    }
}

void CBlueprintEditor2::File_MarkModified()
{
    if (m_Document->m_IsModified)
        return;

    m_Document->m_IsModified = true;

    UpdateTitle();
}

void CBlueprintEditor2::File_New()
{
    if (!File_Close())
        return;

    LOGI("[File] New");

    m_Document = make_unique<Document>();
    m_Blueprint = &m_Document->m_Blueprint;
}

bool CBlueprintEditor2::File_Open(string_view path, string* error)
{
    auto document = make_unique<Document>();
    if (!document->Load(path))
    {
        if (error)
        {
            ImGuiTextBuffer buffer;
            buffer.appendf("Failed to load blueprint from file \"%" PRI_sv "\".", FMT_sv(path));
            *error = buffer.c_str();
        }
        else
            LOGE("Failed to load blueprint from file \"%" PRI_sv "\".", FMT_sv(path));
        return false;
    }

    if (!File_Close())
        return false;

    LOGI("[File] Open \"%" PRI_sv "\"", FMT_sv(path));

    auto mostRecentlyOpenFiles = Application_GetMostRecentlyOpenFileList();
    mostRecentlyOpenFiles.Add(to_string(path));

    document->SetPath(path);

    m_Document = std::move(document);
    m_Blueprint = &m_Document->m_Blueprint;

    UpdateTitle();

    m_Document->OnMakeCurrent();

    return true;
}

bool CBlueprintEditor2::File_Open()
{
    const char* filterPatterns[1] = {"*.bp"};
    auto path = tinyfd_openFileDialog("Open Blueprint...", m_Document ? m_Document->m_Path.c_str() : nullptr, 1,
                                      filterPatterns, "Blueprint Files (*.bp)", 0);
    if (!path)
        return false;

    string error;
    if (!File_Open(path, &error) && !error.empty())
    {
        tinyfd_messageBox((ToString(GetName()) + " - Open Blueprint...").c_str(), error.c_str(), "ok", "error", 1);

        LOGE("%s", error.c_str());

        return false;
    }

    return true;
}

bool CBlueprintEditor2::File_Close()
{
    if (!File_IsOpen())
        return true;

    if (File_IsModified())
    {
        auto dialogResult = tinyfd_messageBox(ToString(GetName()).c_str(),
                                              "Do you want to save changes to this blueprint before closing?",
                                              "yesnocancel", "question", 1);
        if (dialogResult == 1) // yes
        {
            if (!File_Save())
                return false;
        }
        else if (dialogResult == 0) // cancel
        {
            return false;
        }
    }

    LOGI("[File] Close");

    m_Blueprint = nullptr;
    m_Document = nullptr;

    UpdateTitle();

    return true;
}

bool CBlueprintEditor2::File_SaveAsEx(string_view path)
{
    if (!File_IsOpen())
        return true;

    if (!m_Document->Save(path))
    {
        LOGE("Failed to save blueprint to file \"%" PRI_sv "\".", FMT_sv(path));
        return false;
    }

    m_Document->m_IsModified = false;

    LOGI("[File] Save \"%" PRI_sv "\".", FMT_sv(path));

    return true;
}

bool CBlueprintEditor2::File_SaveAs()
{
    const char* filterPatterns[1] = {"*.bp"};
    auto path = tinyfd_saveFileDialog("Save Blueprint...", m_Document->m_Path.c_str(), 1, filterPatterns,
                                      "Blueprint Files (*.bp)");
    if (!path)
        return false;

    if (!File_SaveAsEx(path))
        return false;

    m_Document->SetPath(path);

    UpdateTitle();

    return true;
}

bool CBlueprintEditor2::File_Save()
{
    if (!m_Document->m_Path.empty())
        return File_SaveAsEx(m_Document->m_Path);
    else
        return File_SaveAs();
}

void CBlueprintEditor2::File_Exit()
{
    // if (!Close())
    //     return;

    LOGI("[File] Quit");

    // Quit();
}

void CBlueprintEditor2::Blueprint_Start()
{
    auto entryNode = FindEntryPointNode(*m_Blueprint);

    LOGI("Execution: Start");
    m_Blueprint->Start(*entryNode);
}

void CBlueprintEditor2::Blueprint_Step()
{
    LOGI("Execution: Step %" PRIu32, m_Blueprint->StepCount());
    auto result = m_Blueprint->Step();
    if (result == StepResult::Done)
        LOGI("Execution: Done (%" PRIu32 " steps)", m_Blueprint->StepCount());
    else if (result == StepResult::Error)
        LOGI("Execution: Failed at step %" PRIu32, m_Blueprint->StepCount());
}

void CBlueprintEditor2::Blueprint_Stop()
{
    LOGI("Execution: Stop");
    m_Blueprint->Stop();
}

void CBlueprintEditor2::Blueprint_Run()
{
    auto entryNode = FindEntryPointNode(*m_Blueprint);

    LOGI("Execution: Run");
    auto result = m_Blueprint->Execute(*entryNode);
    if (result == StepResult::Done)
        LOGI("Execution: Done (%" PRIu32 " steps)", m_Blueprint->StepCount());
    else if (result == StepResult::Error)
        LOGI("Execution: Failed at step %" PRIu32, m_Blueprint->StepCount());
}
