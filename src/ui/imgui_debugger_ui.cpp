#include "ui/imgui_debugger_ui.h"
#include "debugger.h"
#include "emulator.h"
#include "disassembler.h"
#include "cpu.h"
#include "memory.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <algorithm>

using namespace videopac;

ImGuiDebuggerUI::ImGuiDebuggerUI(Debugger* debugger, EmulatorCore* emulator,
                                 SDL_Window* window, SDL_Renderer* renderer)
    : debugger_(debugger)
    , emulator_(emulator)
    , window_(window)
    , renderer_(renderer)
    , visible_(false)
    , display_mode_(DisplayMode::Overlay)
    , imgui_context_(nullptr)
{
    // Initialize panel state
    memory_state_ = {};
    memory_state_.current_address = 0;
    memory_state_.goto_address = 0;
    memory_state_.goto_pending = false;
    memory_state_.edit_value = 0;
    memory_state_.edit_address = 0;
    memory_state_.editing = false;
    memory_state_.search_result_index = 0;
    
    disasm_state_ = {};
    disasm_state_.current_address = 0;
    disasm_state_.cursor_address = 0;
    disasm_state_.follow_pc = true;
    
    watch_state_ = {};
    watch_state_.new_watch_buffer[0] = '\0';
    watch_state_.new_label_buffer[0] = '\0';
    watch_state_.show_error = false;
    
    breakpoint_state_ = {};
    breakpoint_state_.address_buffer[0] = '\0';
    breakpoint_state_.condition_buffer[0] = '\0';
    breakpoint_state_.show_add_dialog = false;
    breakpoint_state_.show_error = false;
    
    // Initialize panel visibility (all panels visible by default)
    panel_visibility_ = {};
    panel_visibility_.cpu_state = true;
    panel_visibility_.memory = true;
    panel_visibility_.vdc_registers = true;
    panel_visibility_.breakpoints = true;
    panel_visibility_.disassembly = true;
    panel_visibility_.call_stack = true;
    panel_visibility_.watch = true;
    panel_visibility_.controls = true;
}

ImGuiDebuggerUI::~ImGuiDebuggerUI() {
    shutdown();
}

bool ImGuiDebuggerUI::initialize() {
    // Create ImGui context
    IMGUI_CHECKVERSION();
    imgui_context_ = ImGui::CreateContext();
    if (!imgui_context_) {
        return false;
    }
    
    ImGui::SetCurrentContext(imgui_context_);
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_)) {
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
        return false;
    }
    
    if (!ImGui_ImplSDLRenderer2_Init(renderer_)) {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
        return false;
    }
    
    // Load state when debugger UI is initialized
    load_state();
    
    return true;
}

void ImGuiDebuggerUI::shutdown() {
    // Save state when application shuts down
    save_state();
    
    if (imgui_context_) {
        ImGui::SetCurrentContext(imgui_context_);
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
    }
}

void ImGuiDebuggerUI::show() {
    visible_ = true;
}

void ImGuiDebuggerUI::hide() {
    // Save state when debugger UI is closed
    save_state();
    
    visible_ = false;
}

bool ImGuiDebuggerUI::is_visible() const {
    return visible_;
}

void ImGuiDebuggerUI::render() {
    if (!visible_ || !imgui_context_) {
        return;
    }
    
    // Safety check: ensure emulator and debugger are valid
    if (!emulator_ || !debugger_) {
        return;
    }
    
    // Note: ImGui context is already set in render_frame() before NewFrame()
    // No need to call ImGui::SetCurrentContext() here
    
    // Wrap all rendering in try-catch to prevent crashes
    try {
        // Handle keyboard shortcuts
        handle_shortcuts();
        
        // Render View menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                // Panel visibility toggles
                ImGui::MenuItem("CPU State", nullptr, &panel_visibility_.cpu_state);
                ImGui::MenuItem("Memory Viewer", nullptr, &panel_visibility_.memory);
                ImGui::MenuItem("VDC Registers", nullptr, &panel_visibility_.vdc_registers);
                ImGui::MenuItem("Breakpoints", nullptr, &panel_visibility_.breakpoints);
                ImGui::MenuItem("Disassembly", nullptr, &panel_visibility_.disassembly);
                ImGui::MenuItem("Call Stack", nullptr, &panel_visibility_.call_stack);
                ImGui::MenuItem("Watch Expressions", nullptr, &panel_visibility_.watch);
                ImGui::MenuItem("Controls", nullptr, &panel_visibility_.controls);
                
                ImGui::Separator();
                
                // Display mode selector
                ImGui::Text("Display Mode:");
                if (ImGui::RadioButton("Overlay", display_mode_ == DisplayMode::Overlay)) {
                    display_mode_ = DisplayMode::Overlay;
                }
                if (ImGui::RadioButton("Split Screen", display_mode_ == DisplayMode::SplitScreen)) {
                    display_mode_ = DisplayMode::SplitScreen;
                }
                
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        // Render panels (conditionally based on visibility)
        if (panel_visibility_.cpu_state) {
            render_cpu_state_panel();
        }
        if (panel_visibility_.memory) {
            render_memory_panel();
        }
        if (panel_visibility_.vdc_registers) {
            render_vdc_registers_panel();
        }
        if (panel_visibility_.breakpoints) {
            render_breakpoints_panel();
        }
        if (panel_visibility_.controls) {
            render_controls_panel();
        }
        if (panel_visibility_.watch) {
            render_watch_panel();
        }
        if (panel_visibility_.disassembly) {
            render_disassembly_panel();
        }
        if (panel_visibility_.call_stack) {
            render_call_stack_panel();
        }
    } catch (const std::exception& e) {
        // Log error and display error panel
        fprintf(stderr, "[ERROR] Exception in render(): %s\n", e.what());
        
        // Display error message in a simple window
        if (ImGui::Begin("Debugger Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error rendering debugger:");
            ImGui::Text("%s", e.what());
            if (ImGui::Button("Close Debugger")) {
                hide();
            }
            ImGui::End();
        }
    } catch (...) {
        // Catch any unknown exceptions
        fprintf(stderr, "[ERROR] Unknown exception in render()\n");
        
        // Display error message
        if (ImGui::Begin("Debugger Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Unknown error occurred");
            if (ImGui::Button("Close Debugger")) {
                hide();
            }
            ImGui::End();
        }
    }
}

void ImGuiDebuggerUI::process_event(const SDL_Event& event) {
    if (!visible_ || !imgui_context_) {
        return;
    }
    
    ImGui::SetCurrentContext(imgui_context_);
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void ImGuiDebuggerUI::set_display_mode(DisplayMode mode) {
    display_mode_ = mode;
}

DisplayMode ImGuiDebuggerUI::get_display_mode() const {
    return display_mode_;
}

void ImGuiDebuggerUI::save_state() {
    try {
        // Create JSON object for custom state
        std::string json = "{\n";
    
    // Serialize breakpoints array
    json += "  \"breakpoints\": [\n";
    const std::vector<Breakpoint>& breakpoints = debugger_->get_breakpoints();
    for (size_t i = 0; i < breakpoints.size(); i++) {
        const Breakpoint& bp = breakpoints[i];
        json += "    {\n";
        json += "      \"address\": " + std::to_string(bp.address) + ",\n";
        
        // Escape condition string for JSON
        std::string escaped_condition;
        for (char c : bp.condition) {
            if (c == '"') {
                escaped_condition += "\\\"";
            } else if (c == '\\') {
                escaped_condition += "\\\\";
            } else if (c == '\n') {
                escaped_condition += "\\n";
            } else if (c == '\r') {
                escaped_condition += "\\r";
            } else if (c == '\t') {
                escaped_condition += "\\t";
            } else {
                escaped_condition += c;
            }
        }
        
        json += "      \"condition\": \"" + escaped_condition + "\",\n";
        json += "      \"enabled\": " + std::string(bp.enabled ? "true" : "false") + ",\n";
        json += "      \"has_condition\": " + std::string(bp.has_condition ? "true" : "false") + ",\n";
        json += "      \"condition_only\": " + std::string(bp.condition_only ? "true" : "false") + "\n";
        json += "    }";
        
        if (i < breakpoints.size() - 1) {
            json += ",";
        }
        json += "\n";
    }
    json += "  ],\n";
    
    // Serialize watch expressions array
    json += "  \"watch_expressions\": [\n";
    for (size_t i = 0; i < watch_state_.expressions.size(); i++) {
        const WatchExpression& watch = watch_state_.expressions[i];
        json += "    {\n";
        
        // Type
        json += "      \"type\": \"" + std::string(watch.type == WatchExpression::Type::MemoryAddress ? "memory" : "register") + "\",\n";
        
        // Escape expression string for JSON
        std::string escaped_expression;
        for (char c : watch.expression) {
            if (c == '"') {
                escaped_expression += "\\\"";
            } else if (c == '\\') {
                escaped_expression += "\\\\";
            } else {
                escaped_expression += c;
            }
        }
        json += "      \"expression\": \"" + escaped_expression + "\",\n";
        
        // Escape label string for JSON
        std::string escaped_label;
        for (char c : watch.label) {
            if (c == '"') {
                escaped_label += "\\\"";
            } else if (c == '\\') {
                escaped_label += "\\\\";
            } else {
                escaped_label += c;
            }
        }
        json += "      \"label\": \"" + escaped_label + "\"\n";
        
        json += "    }";
        
        if (i < watch_state_.expressions.size() - 1) {
            json += ",";
        }
        json += "\n";
    }
    json += "  ],\n";
    
    // Serialize display mode
    json += "  \"display_mode\": \"" + std::string(display_mode_ == DisplayMode::Overlay ? "overlay" : "split_screen") + "\",\n";
    
    // Serialize panel visibility flags
    json += "  \"panel_visibility\": {\n";
    json += "    \"cpu_state\": " + std::string(panel_visibility_.cpu_state ? "true" : "false") + ",\n";
    json += "    \"memory\": " + std::string(panel_visibility_.memory ? "true" : "false") + ",\n";
    json += "    \"vdc_registers\": " + std::string(panel_visibility_.vdc_registers ? "true" : "false") + ",\n";
    json += "    \"breakpoints\": " + std::string(panel_visibility_.breakpoints ? "true" : "false") + ",\n";
    json += "    \"disassembly\": " + std::string(panel_visibility_.disassembly ? "true" : "false") + ",\n";
    json += "    \"call_stack\": " + std::string(panel_visibility_.call_stack ? "true" : "false") + ",\n";
    json += "    \"watch\": " + std::string(panel_visibility_.watch ? "true" : "false") + ",\n";
    json += "    \"controls\": " + std::string(panel_visibility_.controls ? "true" : "false") + "\n";
    json += "  }\n";
    
    json += "}\n";
    
    // Write JSON to file
    FILE* file = fopen("debugger_state.json", "w");
    if (!file) {
        // Catch file I/O exceptions - log error if save fails
        fprintf(stderr, "Error: Failed to open debugger_state.json for writing\n");
        return;
    }
    
    size_t bytes_written = fwrite(json.c_str(), 1, json.size(), file);
    fclose(file);
    
    if (bytes_written != json.size()) {
        // Catch file I/O exceptions - log error if save fails
        fprintf(stderr, "Error: Failed to write complete debugger state to file\n");
        return;
    }
    
    // Note: ImGui automatically saves window positions/sizes to imgui.ini
    } catch (const std::exception& e) {
        // Catch any other exceptions during save
        fprintf(stderr, "Error: Exception during save_state: %s\n", e.what());
    } catch (...) {
        // Catch any unknown exceptions
        fprintf(stderr, "Error: Unknown exception during save_state\n");
    }
}

void ImGuiDebuggerUI::load_state() {
    try {
        // Read JSON from file
        FILE* file = fopen("debugger_state.json", "r");
        if (!file) {
            // Handle missing file gracefully (use defaults)
            // This is not an error - just means first run or file was deleted
            return;
        }
        
        // Read entire file into string
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        if (file_size <= 0) {
            fclose(file);
            // Empty file, use defaults
            return;
        }
        
        std::string json;
        json.resize(file_size);
        size_t bytes_read = fread(&json[0], 1, file_size, file);
        fclose(file);
        
        if (bytes_read != static_cast<size_t>(file_size)) {
            // Handle corrupted file gracefully (use defaults, log error)
            fprintf(stderr, "Warning: Failed to read debugger state file completely, using defaults\n");
            return;
        }
    
        // Simple manual JSON parsing
        // Parse breakpoints array
        size_t breakpoints_pos = json.find("\"breakpoints\":");
        if (breakpoints_pos != std::string::npos) {
            size_t array_start = json.find('[', breakpoints_pos);
            size_t array_end = json.find(']', array_start);
            
            if (array_start != std::string::npos && array_end != std::string::npos) {
                // Clear existing breakpoints
                debugger_->clear_all_breakpoints();
                
                // Parse each breakpoint object
                size_t pos = array_start + 1;
                while (pos < array_end) {
                    size_t obj_start = json.find('{', pos);
                    if (obj_start == std::string::npos || obj_start >= array_end) break;
                    
                    size_t obj_end = json.find('}', obj_start);
                    if (obj_end == std::string::npos || obj_end >= array_end) break;
                    
                    std::string obj = json.substr(obj_start, obj_end - obj_start + 1);
                    
                    // Parse address
                    uint16 address = 0;
                    size_t addr_pos = obj.find("\"address\":");
                    if (addr_pos != std::string::npos) {
                        size_t num_start = obj.find_first_of("0123456789", addr_pos);
                        if (num_start != std::string::npos) {
                            address = static_cast<uint16>(std::stoi(obj.substr(num_start)));
                        }
                    }
                    
                    // Parse condition
                    std::string condition;
                    size_t cond_pos = obj.find("\"condition\":");
                    if (cond_pos != std::string::npos) {
                        size_t str_start = obj.find('"', cond_pos + 12);
                        size_t str_end = obj.find('"', str_start + 1);
                        if (str_start != std::string::npos && str_end != std::string::npos) {
                            condition = obj.substr(str_start + 1, str_end - str_start - 1);
                            
                            // Unescape JSON string
                            std::string unescaped;
                            for (size_t i = 0; i < condition.size(); i++) {
                                if (condition[i] == '\\' && i + 1 < condition.size()) {
                                    char next = condition[i + 1];
                                    if (next == '"') {
                                        unescaped += '"';
                                        i++;
                                    } else if (next == '\\') {
                                        unescaped += '\\';
                                        i++;
                                    } else if (next == 'n') {
                                        unescaped += '\n';
                                        i++;
                                    } else if (next == 'r') {
                                        unescaped += '\r';
                                        i++;
                                    } else if (next == 't') {
                                        unescaped += '\t';
                                        i++;
                                    } else {
                                        unescaped += condition[i];
                                    }
                                } else {
                                    unescaped += condition[i];
                                }
                            }
                            condition = unescaped;
                        }
                    }
                    
                    // Parse enabled
                    bool enabled = true;
                    size_t enabled_pos = obj.find("\"enabled\":");
                    if (enabled_pos != std::string::npos) {
                        size_t true_pos = obj.find("true", enabled_pos);
                        size_t false_pos = obj.find("false", enabled_pos);
                        if (false_pos != std::string::npos && 
                            (true_pos == std::string::npos || false_pos < true_pos)) {
                            enabled = false;
                        }
                    }
                    
                    // Parse condition_only
                    bool condition_only = false;
                    size_t cond_only_pos = obj.find("\"condition_only\":");
                    if (cond_only_pos != std::string::npos) {
                        size_t true_pos = obj.find("true", cond_only_pos);
                        if (true_pos != std::string::npos && 
                            true_pos < cond_only_pos + 30) {
                            condition_only = true;
                        }
                    }
                    
                    // Add breakpoint to debugger
                    if (condition_only) {
                        debugger_->add_breakpoint(condition);
                    } else if (!condition.empty()) {
                        debugger_->add_breakpoint(address, condition);
                    } else {
                        debugger_->add_breakpoint(address);
                    }
                    
                    // Set enabled state if needed
                    if (!enabled && !condition_only) {
                        debugger_->enable_breakpoint(address, false);
                    }
                    
                    pos = obj_end + 1;
                }
            }
        }
        
        // Parse watch expressions array
        size_t watch_pos = json.find("\"watch_expressions\":");
        if (watch_pos != std::string::npos) {
            size_t array_start = json.find('[', watch_pos);
            size_t array_end = json.find(']', array_start);
            
            if (array_start != std::string::npos && array_end != std::string::npos) {
                // Clear existing watch expressions
                watch_state_.expressions.clear();
                
                // Parse each watch expression object
                size_t pos = array_start + 1;
                while (pos < array_end) {
                    size_t obj_start = json.find('{', pos);
                    if (obj_start == std::string::npos || obj_start >= array_end) break;
                    
                    size_t obj_end = json.find('}', obj_start);
                    if (obj_end == std::string::npos || obj_end >= array_end) break;
                    
                    std::string obj = json.substr(obj_start, obj_end - obj_start + 1);
                    
                    WatchExpression watch;
                    watch.last_value = 0;
                    watch.changed = false;
                    
                    // Parse type
                    size_t type_pos = obj.find("\"type\":");
                    if (type_pos != std::string::npos) {
                        size_t str_start = obj.find('"', type_pos + 7);
                        size_t str_end = obj.find('"', str_start + 1);
                        if (str_start != std::string::npos && str_end != std::string::npos) {
                            std::string type_str = obj.substr(str_start + 1, str_end - str_start - 1);
                            if (type_str == "memory") {
                                watch.type = WatchExpression::Type::MemoryAddress;
                            } else {
                                watch.type = WatchExpression::Type::Register;
                            }
                        }
                    }
                    
                    // Parse expression
                    size_t expr_pos = obj.find("\"expression\":");
                    if (expr_pos != std::string::npos) {
                        size_t str_start = obj.find('"', expr_pos + 13);
                        size_t str_end = obj.find('"', str_start + 1);
                        if (str_start != std::string::npos && str_end != std::string::npos) {
                            watch.expression = obj.substr(str_start + 1, str_end - str_start - 1);
                            
                            // Unescape JSON string
                            std::string unescaped;
                            for (size_t i = 0; i < watch.expression.size(); i++) {
                                if (watch.expression[i] == '\\' && i + 1 < watch.expression.size()) {
                                    char next = watch.expression[i + 1];
                                    if (next == '"') {
                                        unescaped += '"';
                                        i++;
                                    } else if (next == '\\') {
                                        unescaped += '\\';
                                        i++;
                                    } else {
                                        unescaped += watch.expression[i];
                                    }
                                } else {
                                    unescaped += watch.expression[i];
                                }
                            }
                            watch.expression = unescaped;
                        }
                    }
                    
                    // Parse label
                    size_t label_pos = obj.find("\"label\":");
                    if (label_pos != std::string::npos) {
                        size_t str_start = obj.find('"', label_pos + 8);
                        size_t str_end = obj.find('"', str_start + 1);
                        if (str_start != std::string::npos && str_end != std::string::npos) {
                            watch.label = obj.substr(str_start + 1, str_end - str_start - 1);
                            
                            // Unescape JSON string
                            std::string unescaped;
                            for (size_t i = 0; i < watch.label.size(); i++) {
                                if (watch.label[i] == '\\' && i + 1 < watch.label.size()) {
                                    char next = watch.label[i + 1];
                                    if (next == '"') {
                                        unescaped += '"';
                                        i++;
                                    } else if (next == '\\') {
                                        unescaped += '\\';
                                        i++;
                                    } else {
                                        unescaped += watch.label[i];
                                    }
                                } else {
                                    unescaped += watch.label[i];
                                }
                            }
                            watch.label = unescaped;
                        }
                    }
                    
                    // Add to watch list
                    watch_state_.expressions.push_back(watch);
                    
                    pos = obj_end + 1;
                }
            }
        }
        
        // Parse display mode
        size_t display_mode_pos = json.find("\"display_mode\":");
        if (display_mode_pos != std::string::npos) {
            size_t str_start = json.find('"', display_mode_pos + 15);
            size_t str_end = json.find('"', str_start + 1);
            if (str_start != std::string::npos && str_end != std::string::npos) {
                std::string mode_str = json.substr(str_start + 1, str_end - str_start - 1);
                if (mode_str == "overlay") {
                    display_mode_ = DisplayMode::Overlay;
                } else if (mode_str == "split_screen") {
                    display_mode_ = DisplayMode::SplitScreen;
                }
            }
        }
        
        // Parse panel visibility flags
        size_t panel_vis_pos = json.find("\"panel_visibility\":");
        if (panel_vis_pos != std::string::npos) {
            size_t obj_start = json.find('{', panel_vis_pos);
            size_t obj_end = json.find('}', obj_start);
            
            if (obj_start != std::string::npos && obj_end != std::string::npos) {
                std::string obj = json.substr(obj_start, obj_end - obj_start + 1);
                
                // Helper lambda to parse boolean value
                auto parse_bool = [&obj](const char* key) -> bool {
                    std::string search_key = std::string("\"") + key + "\":";
                    size_t key_pos = obj.find(search_key);
                    if (key_pos != std::string::npos) {
                        size_t true_pos = obj.find("true", key_pos);
                        size_t false_pos = obj.find("false", key_pos);
                        if (true_pos != std::string::npos && 
                            (false_pos == std::string::npos || true_pos < false_pos) &&
                            true_pos < key_pos + 30) {
                            return true;
                        }
                    }
                    return false;
                };
                
                panel_visibility_.cpu_state = parse_bool("cpu_state");
                panel_visibility_.memory = parse_bool("memory");
                panel_visibility_.vdc_registers = parse_bool("vdc_registers");
                panel_visibility_.breakpoints = parse_bool("breakpoints");
                panel_visibility_.disassembly = parse_bool("disassembly");
                panel_visibility_.call_stack = parse_bool("call_stack");
                panel_visibility_.watch = parse_bool("watch");
                panel_visibility_.controls = parse_bool("controls");
            }
        }
        
    } catch (const std::exception& e) {
        // Catch JSON parsing exceptions - display warning if load fails, use defaults
        fprintf(stderr, "Warning: Exception while parsing debugger state file: %s, using defaults\n", e.what());
    } catch (...) {
        // Handle corrupted JSON gracefully (use defaults, log error)
        fprintf(stderr, "Warning: Failed to parse debugger state file, using defaults\n");
    }
    
    // Note: ImGui automatically loads window positions/sizes from imgui.ini
}

// Panel rendering methods (to be implemented in later tasks)
void ImGuiDebuggerUI::render_cpu_state_panel() {
    // In split screen mode, position window in right half
    if (display_mode_ == DisplayMode::SplitScreen) {
        ImGuiIO& io = ImGui::GetIO();
        float right_half_x = io.DisplaySize.x / 2.0f;
        ImGui::SetNextWindowPos(ImVec2(right_half_x, 20), ImGuiCond_FirstUseEver);
    }
    
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("CPU State")) {
        ImGui::End();
        return;
    }
    
    // Get CPU state from emulator
    CPUState cpu_state = emulator_->get_cpu_state();
    
    // Display PC in hexadecimal format
    ImGui::Text("PC: 0x%04X", cpu_state.pc);
    ImGui::SameLine();
    
    // Display accumulator (A) in hexadecimal format
    ImGui::Text("A: 0x%02X", cpu_state.a);
    ImGui::SameLine();
    
    // Display PSW in hexadecimal and binary formats
    ImGui::Text("PSW: 0x%02X (0b%c%c%c%c%c%c%c%c)",
                cpu_state.psw,
                (cpu_state.psw & 0x80) ? '1' : '0',
                (cpu_state.psw & 0x40) ? '1' : '0',
                (cpu_state.psw & 0x20) ? '1' : '0',
                (cpu_state.psw & 0x10) ? '1' : '0',
                (cpu_state.psw & 0x08) ? '1' : '0',
                (cpu_state.psw & 0x04) ? '1' : '0',
                (cpu_state.psw & 0x02) ? '1' : '0',
                (cpu_state.psw & 0x01) ? '1' : '0');
    
    // Display stack pointer (SP) value
    ImGui::Text("SP: %d", cpu_state.sp);
    ImGui::SameLine();
    
    // Display current register bank indicator (0 or 1)
    ImGui::Text("Bank: %d", cpu_state.current_bank);
    ImGui::SameLine();
    
    // Display F1 flag
    ImGui::Text("F1: %d", cpu_state.f1_flag ? 1 : 0);
    ImGui::SameLine();
    
    // Display DBF flag (memory bank)
    ImGui::Text("DBF: %d", cpu_state.memory_bank ? 1 : 0);
    
    ImGui::Separator();
    
    // Display individual PSW flags
    ImGui::Text("Flags:");
    ImGui::SameLine();
    
    // Carry flag (bit 7)
    bool carry = (cpu_state.psw & 0x80) != 0;
    ImGui::Text("[%c] C", carry ? 'X' : ' ');
    ImGui::SameLine();
    
    // Auxiliary Carry flag (bit 6)
    bool aux_carry = (cpu_state.psw & 0x40) != 0;
    ImGui::Text("[%c] AC", aux_carry ? 'X' : ' ');
    ImGui::SameLine();
    
    // F0 flag (bit 5)
    bool f0 = (cpu_state.psw & 0x20) != 0;
    ImGui::Text("[%c] F0", f0 ? 'X' : ' ');
    ImGui::SameLine();
    
    // Register Bank Select flag (bit 4)
    bool bs = (cpu_state.psw & 0x10) != 0;
    ImGui::Text("[%c] BS", bs ? 'X' : ' ');
    
    ImGui::Separator();
    
    // Display all working registers (R0-R7) in hexadecimal format
    ImGui::Text("Registers:");
    for (int i = 0; i < 8; i++) {
        // Get register from current bank
        uint8 reg_index = cpu_state.current_bank * 8 + i;
        ImGui::Text("R%d: 0x%02X", i, cpu_state.r[reg_index]);
        if (i < 7) {
            ImGui::SameLine();
        }
    }
    
    ImGui::Separator();
    
    // Get current instruction disassembly from Disassembler
    Disassembler disasm;
    MemoryState mem_state = emulator_->get_memory_state();
    
    // Get memory pointer for disassembly - use cart_rom if available, otherwise bios_rom
    const uint8* memory_ptr = nullptr;
    if (!mem_state.cart_rom.empty() && cpu_state.pc < mem_state.cart_rom.size()) {
        memory_ptr = mem_state.cart_rom.data();
    } else if (cpu_state.pc < 1024) {
        memory_ptr = mem_state.bios_rom;
    }
    
    // Display disassembled current instruction
    ImGui::Text("Current Instruction:");
    if (memory_ptr) {
        Instruction instr = disasm.disassemble_instruction(cpu_state.pc, memory_ptr);
        std::string disassembly = disasm.format_instruction(instr);
        ImGui::Text("  %s", disassembly.c_str());
    } else {
        ImGui::Text("  (no memory loaded)");
    }
    
    ImGui::End();
}

void ImGuiDebuggerUI::render_memory_panel() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Memory Viewer")) {
        ImGui::End();
        return;
    }
    
    // Get memory state from emulator
    MemoryState mem_state = emulator_->get_memory_state();
    
    // Goto address functionality
    static char goto_buffer[16] = "";
    static bool goto_error = false;
    static std::string goto_error_msg;
    
    ImGui::Text("Goto Address:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText("##goto", goto_buffer, sizeof(goto_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        // Parse hex address
        unsigned int addr;
        if (sscanf(goto_buffer, "%x", &addr) == 1) {
            // Validate address is in range 0x0000-0xFFFF
            if (addr <= 0xFFFF) {
                memory_state_.current_address = static_cast<uint16>(addr);
                memory_state_.goto_pending = true;
                goto_error = false;
            } else {
                // Display error message if invalid
                goto_error = true;
                goto_error_msg = "Address out of range (must be 0x0000-0xFFFF)";
            }
        } else {
            // Invalid hex format
            goto_error = true;
            goto_error_msg = "Invalid hex address format";
        }
    }
    
    // Display error message using ImGui::TextColored() if invalid
    if (goto_error) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", goto_error_msg.c_str());
    }
    
    // Search functionality
    static char search_buffer[256] = "";
    ImGui::SameLine();
    ImGui::Text("Search:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::InputText("##search", search_buffer, sizeof(search_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        memory_state_.search_query = search_buffer;
        memory_state_.search_results.clear();
        memory_state_.search_result_index = 0;
        
        // Parse search query as hex bytes
        std::vector<uint8> search_bytes;
        const char* p = search_buffer;
        while (*p) {
            // Skip whitespace
            while (*p == ' ' || *p == '\t') p++;
            if (!*p) break;
            
            // Parse hex byte
            unsigned int byte_val;
            if (sscanf(p, "%2x", &byte_val) == 1) {
                search_bytes.push_back(static_cast<uint8>(byte_val));
                p += 2;
            } else {
                break;
            }
        }
        
        // Search in all memory regions
        if (!search_bytes.empty()) {
            // Search in BIOS ROM
            for (size_t i = 0; i <= sizeof(mem_state.bios_rom) - search_bytes.size(); i++) {
                bool match = true;
                for (size_t j = 0; j < search_bytes.size(); j++) {
                    if (mem_state.bios_rom[i + j] != search_bytes[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    memory_state_.search_results.push_back(static_cast<uint16>(i));
                }
            }
            
            // Search in cartridge ROM
            for (size_t i = 0; i <= mem_state.cart_rom.size() - search_bytes.size(); i++) {
                bool match = true;
                for (size_t j = 0; j < search_bytes.size(); j++) {
                    if (mem_state.cart_rom[i + j] != search_bytes[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    memory_state_.search_results.push_back(static_cast<uint16>(0x400 + i));
                }
            }
            
            // Search in external RAM
            for (size_t i = 0; i <= sizeof(mem_state.external_ram) - search_bytes.size(); i++) {
                bool match = true;
                for (size_t j = 0; j < search_bytes.size(); j++) {
                    if (mem_state.external_ram[i + j] != search_bytes[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    memory_state_.search_results.push_back(static_cast<uint16>(0xF000 + i));
                }
            }
        }
    }
    
    // Display search results
    if (!memory_state_.search_results.empty()) {
        ImGui::SameLine();
        ImGui::Text("Found: %zu", memory_state_.search_results.size());
        if (memory_state_.search_result_index < static_cast<int>(memory_state_.search_results.size())) {
            ImGui::SameLine();
            if (ImGui::Button("Next")) {
                memory_state_.current_address = memory_state_.search_results[memory_state_.search_result_index];
                memory_state_.goto_pending = true;
                memory_state_.search_result_index = (memory_state_.search_result_index + 1) % memory_state_.search_results.size();
            }
        }
    }
    
    ImGui::Separator();
    
    // Scrollable memory view
    ImGui::BeginChild("MemoryScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // Track modified bytes (for now, we'll implement basic tracking)
    static std::vector<uint16> modified_addresses;
    
    // Display memory in hex dump format with 16 bytes per row
    const int bytes_per_row = 16;
    const int total_rows = 0x10000 / bytes_per_row;  // 64KB address space
    
    // Use clipper for efficient rendering of large lists
    ImGuiListClipper clipper;
    clipper.Begin(total_rows);
    
    // Handle goto pending
    if (memory_state_.goto_pending) {
        int target_row = memory_state_.current_address / bytes_per_row;
        ImGui::SetScrollY(target_row * ImGui::GetTextLineHeightWithSpacing());
        memory_state_.goto_pending = false;
    }
    
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
            uint16 address = row * bytes_per_row;
            
            // Display address column in hexadecimal format
            ImGui::Text("0x%04X", address);
            ImGui::SameLine();
            
            // Read bytes for this row
            uint8 row_data[16];
            for (int i = 0; i < bytes_per_row; i++) {
                uint16 addr = address + i;
                
                // Determine which memory region this address belongs to
                if (addr < 0x400) {
                    // BIOS ROM (0x0000-0x03FF)
                    row_data[i] = mem_state.bios_rom[addr];
                } else if (addr < 0x400 + mem_state.cart_rom.size()) {
                    // Cartridge ROM (0x0400+)
                    row_data[i] = mem_state.cart_rom[addr - 0x400];
                } else if (addr >= 0xF000 && addr < 0xF080) {
                    // External RAM (0xF000-0xF07F)
                    row_data[i] = mem_state.external_ram[addr - 0xF000];
                } else {
                    // Unmapped memory
                    row_data[i] = 0xFF;
                }
            }
            
            // Display hex values for each byte
            for (int i = 0; i < bytes_per_row; i++) {
                uint16 byte_addr = address + i;
                
                // Check if this byte was modified
                bool is_modified = std::find(modified_addresses.begin(), modified_addresses.end(), byte_addr) != modified_addresses.end();
                
                // Highlight modified bytes in distinct color
                if (is_modified) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));  // Orange
                }
                
                // Click-to-edit functionality
                ImGui::PushID(byte_addr);
                
                // Check if we're in edit mode for this address
                if (memory_state_.editing && memory_state_.edit_address == byte_addr) {
                    // Show input field
                    ImGui::SetNextItemWidth(30);
                    char edit_buffer[4];
                    snprintf(edit_buffer, sizeof(edit_buffer), "%02X", memory_state_.edit_value);
                    
                    if (ImGui::InputText("##edit", edit_buffer, sizeof(edit_buffer), 
                                        ImGuiInputTextFlags_CharsHexadecimal | 
                                        ImGuiInputTextFlags_EnterReturnsTrue | 
                                        ImGuiInputTextFlags_AutoSelectAll)) {
                        // Parse and write the new value
                        unsigned int new_val;
                        if (sscanf(edit_buffer, "%x", &new_val) == 1) {
                            uint8 byte_val = static_cast<uint8>(new_val);
                            
                            // Validate address is in range 0x0000-0xFFFF (implicit for uint16)
                            // Write to appropriate memory region
                            if (byte_addr >= 0xF000 && byte_addr < 0xF080) {
                                // Only allow editing external RAM
                                emulator_->get_cpu().write_memory(byte_addr, byte_val);
                                
                                // Track as modified
                                if (std::find(modified_addresses.begin(), modified_addresses.end(), byte_addr) == modified_addresses.end()) {
                                    modified_addresses.push_back(byte_addr);
                                }
                            }
                        }
                        memory_state_.editing = false;
                    }
                    
                    // Cancel edit on escape
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                        memory_state_.editing = false;
                    }
                } else {
                    // Display as text, clickable to edit
                    char byte_text[4];
                    snprintf(byte_text, sizeof(byte_text), "%02X", row_data[i]);
                    
                    if (ImGui::Selectable(byte_text, false, ImGuiSelectableFlags_None, ImVec2(20, 0))) {
                        // Validate address is in valid range (0x0000-0xFFFF is implicit for uint16)
                        // Only allow editing RAM
                        if (byte_addr >= 0xF000 && byte_addr < 0xF080) {
                            memory_state_.editing = true;
                            memory_state_.edit_address = byte_addr;
                            memory_state_.edit_value = row_data[i];
                        }
                    }
                }
                
                ImGui::PopID();
                
                if (is_modified) {
                    ImGui::PopStyleColor();
                }
                
                if (i < bytes_per_row - 1) {
                    ImGui::SameLine();
                }
            }
            
            // Display ASCII representation column
            ImGui::SameLine();
            ImGui::Text(" ");
            ImGui::SameLine();
            
            char ascii_text[17];
            for (int i = 0; i < bytes_per_row; i++) {
                uint8 byte = row_data[i];
                // Display printable ASCII characters, otherwise show '.'
                ascii_text[i] = (byte >= 32 && byte < 127) ? static_cast<char>(byte) : '.';
            }
            ascii_text[bytes_per_row] = '\0';
            ImGui::Text("%s", ascii_text);
        }
    }
    
    ImGui::EndChild();
    ImGui::End();
}

void ImGuiDebuggerUI::render_vdc_registers_panel() {
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("VDC Registers")) {
        ImGui::End();
        return;
    }
    
    // Get VDC state from emulator
    VDCState vdc_state = emulator_->get_vdc_state();
    
    // Sprite control registers for sprites 0-3
    if (ImGui::TreeNode("Sprites")) {
        // Videopac 16-color palette (RGBI) - matches PALETTE in types.h
        // Low-intensity (0-7): Background/Grid colors
        // High-intensity (8-15): Sprite/Character colors
        static const ImU32 palette_colors[16] = {
            // Low-intensity
            IM_COL32(0, 0, 0, 255),         // 0: Black
            IM_COL32(8, 57, 214, 255),      // 1: Dark Blue
            IM_COL32(0, 156, 24, 255),      // 2: Dark Green
            IM_COL32(0, 189, 222, 255),     // 3: Light Blue
            IM_COL32(198, 0, 8, 255),       // 4: Dark Red
            IM_COL32(206, 16, 181, 255),    // 5: Violet
            IM_COL32(156, 132, 16, 255),    // 6: Orange/Gold
            IM_COL32(206, 206, 206, 255),   // 7: Grey
            // High-intensity
            IM_COL32(73, 73, 73, 255),      // 8: Light Grey
            IM_COL32(73, 73, 255, 255),     // 9: Blue
            IM_COL32(73, 255, 73, 255),     // 10: Green
            IM_COL32(73, 255, 255, 255),    // 11: Cyan
            IM_COL32(255, 73, 73, 255),     // 12: Red
            IM_COL32(255, 73, 255, 255),    // 13: Magenta
            IM_COL32(255, 255, 73, 255),    // 14: Yellow
            IM_COL32(255, 255, 255, 255)    // 15: White
        };
        
        for (int i = 0; i < 4; i++) {
            uint8 base = i * 4;
            uint8 y = vdc_state.registers[base + 0];
            uint8 x = vdc_state.registers[base + 1];
            uint8 color_reg = vdc_state.registers[base + 2];
            
            // Extract color and flags from color register
            uint8 color = (color_reg >> 3) & 0x07;
            bool shift_full = (color_reg & 0x01) != 0;
            bool shift_even = (color_reg & 0x02) != 0;
            bool double_size = (color_reg & 0x04) != 0;
            
            // Get pattern bytes
            uint8 pattern_base = 0x80 + (i * 8);
            
            if (ImGui::TreeNode((void*)(intptr_t)i, "Sprite %d", i)) {
                ImGui::Text("Position: X=%d Y=%d", x, y);
                ImGui::Text("Color: %d", color);
                ImGui::Text("Size: %s", double_size ? "16x16" : "8x8");
                ImGui::Text("Shift: %s", shift_full ? "Full" : (shift_even ? "Even" : "None"));
                
                // Render sprite pattern visualization
                ImGui::Text("Pattern:");
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                
                // Draw sprite pattern (8x8 base pattern)
                float pixel_size = 8.0f;  // Size of each pixel in the visualization
                ImU32 sprite_color = palette_colors[color];
                ImU32 bg_color = IM_COL32(40, 40, 40, 255);  // Dark gray background
                
                for (int row = 0; row < 8; row++) {
                    uint8 pattern = vdc_state.registers[pattern_base + row];
                    
                    for (int col = 0; col < 8; col++) {
                        bool pixel_on = (pattern & (0x80 >> col)) != 0;
                        ImU32 pixel_color = pixel_on ? sprite_color : bg_color;
                        
                        ImVec2 p_min(canvas_pos.x + col * pixel_size, canvas_pos.y + row * pixel_size);
                        ImVec2 p_max(p_min.x + pixel_size, p_min.y + pixel_size);
                        
                        draw_list->AddRectFilled(p_min, p_max, pixel_color);
                        draw_list->AddRect(p_min, p_max, IM_COL32(80, 80, 80, 255));  // Grid lines
                    }
                }
                
                // Reserve space for the sprite visualization
                ImGui::Dummy(ImVec2(8 * pixel_size, 8 * pixel_size));
                
                // Show pattern bytes in hex
                ImGui::Text("Pattern bytes:");
                for (int row = 0; row < 8; row++) {
                    ImGui::Text("  Row %d: 0x%02X", row, vdc_state.registers[pattern_base + row]);
                }
                
                ImGui::TreePop();
            }
            ImGui::Separator();
        }
        ImGui::TreePop();
    }
    
    // VDC control register (0xA0) with decoded bit flags
    if (ImGui::TreeNode("Control Register (0xA0)")) {
        uint8 control = vdc_state.registers[0xA0];
        ImGui::Text("Value: 0x%02X (0b%c%c%c%c%c%c%c%c)",
                   control,
                   (control & 0x80) ? '1' : '0',
                   (control & 0x40) ? '1' : '0',
                   (control & 0x20) ? '1' : '0',
                   (control & 0x10) ? '1' : '0',
                   (control & 0x08) ? '1' : '0',
                   (control & 0x04) ? '1' : '0',
                   (control & 0x02) ? '1' : '0',
                   (control & 0x01) ? '1' : '0');
        
        ImGui::Text("[%c] Display Enable", (control & 0x20) ? 'X' : ' ');
        ImGui::Text("[%c] Grid Enable", (control & 0x08) ? 'X' : ' ');
        ImGui::Text("[%c] Dot Grid", (control & 0x40) ? 'X' : ' ');
        ImGui::Text("[%c] Fill Mode", (control & 0x80) ? 'X' : ' ');
        ImGui::Text("[%c] HBlank Interrupt", (control & 0x01) ? 'X' : ' ');
        ImGui::Text("[%c] Sound Interrupt", (control & 0x04) ? 'X' : ' ');
        ImGui::Text("[%c] Latch Beam Position", (control & 0x02) ? 'X' : ' ');
        ImGui::Text("[%c] External Overlap", (control & 0x10) ? 'X' : ' ');
        ImGui::TreePop();
    }
    
    // VDC status register (0xA1) with decoded bit flags
    if (ImGui::TreeNode("Status Register (0xA1)")) {
        uint8 status = vdc_state.registers[0xA1];
        ImGui::Text("Value: 0x%02X (0b%c%c%c%c%c%c%c%c)",
                   status,
                   (status & 0x80) ? '1' : '0',
                   (status & 0x40) ? '1' : '0',
                   (status & 0x20) ? '1' : '0',
                   (status & 0x10) ? '1' : '0',
                   (status & 0x08) ? '1' : '0',
                   (status & 0x04) ? '1' : '0',
                   (status & 0x02) ? '1' : '0',
                   (status & 0x01) ? '1' : '0');
        
        ImGui::Text("[%c] VBlank", (status & 0x08) ? 'X' : ' ');
        ImGui::Text("[%c] HBlank", (status & 0x01) ? 'X' : ' ');
        ImGui::Text("[%c] Position Strobe", (status & 0x02) ? 'X' : ' ');
        ImGui::Text("[%c] Sound Needs Service", (status & 0x04) ? 'X' : ' ');
        ImGui::Text("[%c] Character Overlap", (status & 0x80) ? 'X' : ' ');
        ImGui::Text("[%c] External Overlap", (status & 0x40) ? 'X' : ' ');
        ImGui::TreePop();
    }
    
    // Collision register (0xA2) with decoded collision bits
    if (ImGui::TreeNode("Collision Register (0xA2)")) {
        uint8 collision = vdc_state.registers[0xA2];
        ImGui::Text("Value: 0x%02X (0b%c%c%c%c%c%c%c%c)",
                   collision,
                   (collision & 0x80) ? '1' : '0',
                   (collision & 0x40) ? '1' : '0',
                   (collision & 0x20) ? '1' : '0',
                   (collision & 0x10) ? '1' : '0',
                   (collision & 0x08) ? '1' : '0',
                   (collision & 0x04) ? '1' : '0',
                   (collision & 0x02) ? '1' : '0',
                   (collision & 0x01) ? '1' : '0');
        
        ImGui::Text("[%c] Sprite 0", (collision & 0x01) ? 'X' : ' ');
        ImGui::Text("[%c] Sprite 1", (collision & 0x02) ? 'X' : ' ');
        ImGui::Text("[%c] Sprite 2", (collision & 0x04) ? 'X' : ' ');
        ImGui::Text("[%c] Sprite 3", (collision & 0x08) ? 'X' : ' ');
        ImGui::Text("[%c] Vertical Grid", (collision & 0x10) ? 'X' : ' ');
        ImGui::Text("[%c] Horizontal Grid", (collision & 0x20) ? 'X' : ' ');
        ImGui::Text("[%c] Characters", (collision & 0x80) ? 'X' : ' ');
        ImGui::Text("[%c] External Collision", (collision & 0x40) ? 'X' : ' ');
        ImGui::TreePop();
    }
    
    // Color register (0xA3) value
    if (ImGui::TreeNode("Color Register (0xA3)")) {
        uint8 color = vdc_state.registers[0xA3];
        ImGui::Text("Value: 0x%02X", color);
        ImGui::TreePop();
    }
    
    // Beam position registers (X and Y)
    if (ImGui::TreeNode("Beam Position")) {
        uint8 beam_x_reg = vdc_state.registers[0xA4];
        uint8 beam_y_reg = vdc_state.registers[0xA5];
        ImGui::Text("X (0xA4): %d (0x%02X)", beam_x_reg, beam_x_reg);
        ImGui::Text("Y (0xA5): %d (0x%02X)", beam_y_reg, beam_y_reg);
        ImGui::Text("Actual Beam: X=%d Y=%d", vdc_state.beam_x, vdc_state.beam_y);
        ImGui::TreePop();
    }
    
    // Audio registers (shift register bytes, control)
    if (ImGui::TreeNode("Audio")) {
        uint8 sound0 = vdc_state.registers[0xA7];
        uint8 sound1 = vdc_state.registers[0xA8];
        uint8 sound2 = vdc_state.registers[0xA9];
        uint8 sound_ctrl = vdc_state.registers[0xAA];
        
        ImGui::Text("Shift Register:");
        ImGui::Text("  Byte 0 (0xA7): 0x%02X", sound0);
        ImGui::Text("  Byte 1 (0xA8): 0x%02X", sound1);
        ImGui::Text("  Byte 2 (0xA9): 0x%02X", sound2);
        ImGui::Text("  Combined: 0x%06X", (sound2 << 16) | (sound1 << 8) | sound0);
        
        ImGui::Separator();
        ImGui::Text("Control (0xAA): 0x%02X", sound_ctrl);
        
        uint8 volume = sound_ctrl & 0x0F;
        bool noise = (sound_ctrl & 0x10) != 0;
        bool high_freq = (sound_ctrl & 0x20) != 0;
        bool loop = (sound_ctrl & 0x40) != 0;
        bool enabled = (sound_ctrl & 0x80) != 0;
        
        ImGui::Text("  Volume: %d", volume);
        ImGui::Text("  Frequency: %d Hz", high_freq ? 3933 : 983);
        ImGui::Text("  [%c] Enabled", enabled ? 'X' : ' ');
        ImGui::Text("  [%c] Loop", loop ? 'X' : ' ');
        ImGui::Text("  [%c] Noise", noise ? 'X' : ' ');
        ImGui::TreePop();
    }
    
    // Grid control registers with visual representation
    if (ImGui::TreeNode("Grid Control")) {
        ImGui::TextWrapped("Grid layout: 9 rows x 9 columns of horizontal bars, 10 columns x 8 rows of vertical bars");
        ImGui::TextWrapped("Bytes = COLUMNS (left to right), Bits = ROWS (top to bottom)");
        ImGui::Separator();
        
        // Horizontal grid visualization
        if (ImGui::TreeNode("Horizontal Bars (C0-C8, D0-D8)")) {
            ImGui::Text("9 rows x 9 columns = 81 segments");
            ImGui::Spacing();
            
            // Visual grid representation
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            float cell_size = 16.0f;
            
            // Draw grid cells
            for (int row = 0; row < 9; row++) {
                for (int col = 0; col < 9; col++) {
                    bool segment_on = false;
                    
                    if (row < 8) {
                        // Rows 0-7: Check bit 'row' of register C0+col
                        segment_on = (vdc_state.registers[0xC0 + col] & (1 << row)) != 0;
                    } else {
                        // Row 8: Check bit 0 of register D0+col
                        segment_on = (vdc_state.registers[0xD0 + col] & 0x01) != 0;
                    }
                    
                    ImVec2 p_min(canvas_pos.x + col * cell_size, canvas_pos.y + row * cell_size);
                    ImVec2 p_max(p_min.x + cell_size, p_min.y + cell_size);
                    
                    ImU32 cell_color = segment_on ? IM_COL32(0, 150, 255, 255) : IM_COL32(40, 40, 40, 255);
                    draw_list->AddRectFilled(p_min, p_max, cell_color);
                    draw_list->AddRect(p_min, p_max, IM_COL32(80, 80, 80, 255));
                }
            }
            
            ImGui::Dummy(ImVec2(9 * cell_size, 9 * cell_size));
            ImGui::Spacing();
            
            // Register values
            ImGui::Text("Registers C0-C8 (rows 0-7):");
            for (int i = 0; i < 9; i++) {
                uint8 grid_val = vdc_state.registers[0xC0 + i];
                ImGui::Text("  Col %d (0xC%X): 0x%02X  %c%c%c%c%c%c%c%c", 
                           i, i, grid_val,
                           (grid_val & 0x80) ? '1' : '0',
                           (grid_val & 0x40) ? '1' : '0',
                           (grid_val & 0x20) ? '1' : '0',
                           (grid_val & 0x10) ? '1' : '0',
                           (grid_val & 0x08) ? '1' : '0',
                           (grid_val & 0x04) ? '1' : '0',
                           (grid_val & 0x02) ? '1' : '0',
                           (grid_val & 0x01) ? '1' : '0');
            }
            
            ImGui::Spacing();
            ImGui::Text("Registers D0-D8 (row 8, bit 0 only):");
            for (int i = 0; i < 9; i++) {
                uint8 grid_val = vdc_state.registers[0xD0 + i];
                ImGui::Text("  Col %d (0xD%X): 0x%02X  [%c]", 
                           i, i, grid_val,
                           (grid_val & 0x01) ? '1' : '0');
            }
            
            ImGui::TreePop();
        }
        
        ImGui::Separator();
        
        // Vertical grid visualization
        if (ImGui::TreeNode("Vertical Bars (E0-E9)")) {
            ImGui::Text("10 columns x 8 rows = 80 segments");
            ImGui::Spacing();
            
            // Visual grid representation
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            float cell_size = 16.0f;
            
            // Draw grid cells
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 10; col++) {
                    bool segment_on = (vdc_state.registers[0xE0 + col] & (1 << row)) != 0;
                    
                    ImVec2 p_min(canvas_pos.x + col * cell_size, canvas_pos.y + row * cell_size);
                    ImVec2 p_max(p_min.x + cell_size, p_min.y + cell_size);
                    
                    ImU32 cell_color = segment_on ? IM_COL32(0, 255, 150, 255) : IM_COL32(40, 40, 40, 255);
                    draw_list->AddRectFilled(p_min, p_max, cell_color);
                    draw_list->AddRect(p_min, p_max, IM_COL32(80, 80, 80, 255));
                }
            }
            
            ImGui::Dummy(ImVec2(10 * cell_size, 8 * cell_size));
            ImGui::Spacing();
            
            // Register values
            ImGui::Text("Registers E0-E9:");
            for (int i = 0; i < 10; i++) {
                uint8 grid_val = vdc_state.registers[0xE0 + i];
                ImGui::Text("  Col %d (0xE%X): 0x%02X  %c%c%c%c%c%c%c%c", 
                           i, (i < 10) ? i : (i - 10 + 'A'), grid_val,
                           (grid_val & 0x80) ? '1' : '0',
                           (grid_val & 0x40) ? '1' : '0',
                           (grid_val & 0x20) ? '1' : '0',
                           (grid_val & 0x10) ? '1' : '0',
                           (grid_val & 0x08) ? '1' : '0',
                           (grid_val & 0x04) ? '1' : '0',
                           (grid_val & 0x02) ? '1' : '0',
                           (grid_val & 0x01) ? '1' : '0');
            }
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
    
    ImGui::End();
}

void ImGuiDebuggerUI::render_breakpoints_panel() {
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Breakpoints")) {
        ImGui::End();
        return;
    }
    
    // Get breakpoints from debugger
    const std::vector<Breakpoint>& breakpoints = debugger_->get_breakpoints();
    
    // Get current PC to highlight active breakpoint
    CPUState cpu_state = emulator_->get_cpu_state();
    uint16 current_pc = cpu_state.pc;
    
    // Add Breakpoint button (disabled if limit reached)
    bool at_limit = breakpoints.size() >= MAX_BREAKPOINTS;
    if (at_limit) {
        ImGui::BeginDisabled();
    }
    
    if (ImGui::Button("Add Breakpoint")) {
        breakpoint_state_.show_add_dialog = true;
        breakpoint_state_.show_error = false;
        breakpoint_state_.error_message.clear();
    }
    
    if (at_limit) {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Limit reached (%zu/%zu)", breakpoints.size(), MAX_BREAKPOINTS);
    } else {
        ImGui::SameLine();
        ImGui::Text("(%zu/%zu)", breakpoints.size(), MAX_BREAKPOINTS);
    }
    
    ImGui::Separator();
    
    // Display breakpoint list using table
    if (ImGui::BeginTable("BreakpointTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Setup columns
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Condition", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();
        
        // Display each breakpoint
        for (size_t i = 0; i < breakpoints.size(); i++) {
            const Breakpoint& bp = breakpoints[i];
            
            ImGui::TableNextRow();
            
            // Highlight if execution is paused at this address
            bool is_at_breakpoint = debugger_->is_paused() && 
                                   !bp.condition_only && 
                                   bp.address == current_pc;
            
            if (is_at_breakpoint) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 
                                      ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.3f, 0.3f)));
            }
            
            // Column 1: Address (or "-" for condition-only)
            ImGui::TableSetColumnIndex(0);
            if (bp.condition_only) {
                ImGui::Text("-");
            } else {
                ImGui::Text("0x%04X", bp.address);
            }
            
            // Column 2: Condition (or "-" for address-only)
            ImGui::TableSetColumnIndex(1);
            if (bp.has_condition) {
                ImGui::Text("%s", bp.condition.c_str());
            } else {
                ImGui::Text("-");
            }
            
            // Column 3: Enabled checkbox
            ImGui::TableSetColumnIndex(2);
            bool enabled = bp.enabled;
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Checkbox("##enabled", &enabled)) {
                // Toggle enabled functionality
                debugger_->enable_breakpoint(bp.address, enabled);
            }
            ImGui::PopID();
            
            // Column 4: Delete button
            ImGui::TableSetColumnIndex(3);
            ImGui::PushID(static_cast<int>(i) + 1000);
            if (ImGui::Button("Delete")) {
                // Delete breakpoint
                debugger_->remove_breakpoint(bp.address);
            }
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
    
    // Add breakpoint dialog
    if (breakpoint_state_.show_add_dialog) {
        ImGui::OpenPopup("Add Breakpoint");
    }
    
    if (ImGui::BeginPopupModal("Add Breakpoint", &breakpoint_state_.show_add_dialog, 
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Add a new breakpoint:");
        ImGui::Separator();
        
        // Address input
        ImGui::Text("Address (hex, optional):");
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##address", breakpoint_state_.address_buffer, 
                        sizeof(breakpoint_state_.address_buffer));
        
        ImGui::Spacing();
        
        // Condition input
        ImGui::Text("Condition (optional):");
        ImGui::SetNextItemWidth(400);
        ImGui::InputText("##condition", breakpoint_state_.condition_buffer, 
                        sizeof(breakpoint_state_.condition_buffer));
        
        ImGui::Spacing();
        ImGui::Text("Examples: A==0xFF, R0>0x80, PSW&0x10");
        
        // Display error message if validation failed
        if (breakpoint_state_.show_error) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", breakpoint_state_.error_message.c_str());
        }
        
        ImGui::Separator();
        
        // Add button
        if (ImGui::Button("Add", ImVec2(120, 0))) {
            bool has_address = breakpoint_state_.address_buffer[0] != '\0';
            bool has_condition = breakpoint_state_.condition_buffer[0] != '\0';
            
            // Check if at least one field is filled
            if (!has_address && !has_condition) {
                breakpoint_state_.show_error = true;
                breakpoint_state_.error_message = "Must specify at least an address or condition";
            }
            // Check breakpoint limit
            else if (breakpoints.size() >= MAX_BREAKPOINTS) {
                breakpoint_state_.show_error = true;
                breakpoint_state_.error_message = "Breakpoint limit reached (max " + std::to_string(MAX_BREAKPOINTS) + ")";
            }
            else {
                // Validate condition if present
                std::string condition_str = breakpoint_state_.condition_buffer;
                std::string validation_error;
                
                if (has_condition && !validate_breakpoint_condition(condition_str, validation_error)) {
                    // Display error message if malformed
                    breakpoint_state_.show_error = true;
                    breakpoint_state_.error_message = validation_error;
                } else {
                    // Validation passed, add the breakpoint
                    bool success = false;
                    
                    if (has_address && has_condition) {
                        // Address + condition breakpoint
                        unsigned int addr;
                        if (sscanf(breakpoint_state_.address_buffer, "%x", &addr) == 1) {
                            debugger_->add_breakpoint(static_cast<uint16>(addr), condition_str);
                            success = true;
                        } else {
                            breakpoint_state_.show_error = true;
                            breakpoint_state_.error_message = "Invalid address format";
                        }
                    } else if (has_address) {
                        // Address-only breakpoint
                        unsigned int addr;
                        if (sscanf(breakpoint_state_.address_buffer, "%x", &addr) == 1) {
                            debugger_->add_breakpoint(static_cast<uint16>(addr));
                            success = true;
                        } else {
                            breakpoint_state_.show_error = true;
                            breakpoint_state_.error_message = "Invalid address format";
                        }
                    } else {
                        // Condition-only breakpoint
                        debugger_->add_breakpoint(condition_str);
                        success = true;
                    }
                    
                    // Clear buffers and close dialog if successful
                    if (success) {
                        breakpoint_state_.address_buffer[0] = '\0';
                        breakpoint_state_.condition_buffer[0] = '\0';
                        breakpoint_state_.show_add_dialog = false;
                        breakpoint_state_.show_error = false;
                        breakpoint_state_.error_message.clear();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        }
        
        ImGui::SameLine();
        
        // Cancel button
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            breakpoint_state_.address_buffer[0] = '\0';
            breakpoint_state_.condition_buffer[0] = '\0';
            breakpoint_state_.show_add_dialog = false;
            breakpoint_state_.show_error = false;
            breakpoint_state_.error_message.clear();
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void ImGuiDebuggerUI::render_disassembly_panel() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Disassembly")) {
        ImGui::End();
        return;
    }
    
    // Get current CPU state
    CPUState cpu_state = emulator_->get_cpu_state();
    uint16 current_pc = cpu_state.pc;
    
    // Get memory state for disassembly
    MemoryState mem_state = emulator_->get_memory_state();
    
    // Determine which memory region to use for disassembly
    const uint8* memory_ptr = nullptr;
    
    if (!mem_state.cart_rom.empty() && current_pc >= 0x400) {
        memory_ptr = mem_state.cart_rom.data();
    } else if (current_pc < 0x400) {
        memory_ptr = mem_state.bios_rom;
    }
    
    if (!memory_ptr) {
        ImGui::Text("No memory loaded for disassembly");
        ImGui::End();
        return;
    }
    
    // Follow PC mode toggle
    ImGui::Checkbox("Follow PC", &disasm_state_.follow_pc);
    ImGui::SameLine();
    
    // Display current PC
    ImGui::Text("PC: 0x%04X", current_pc);
    ImGui::SameLine();
    
    // Display cursor address
    ImGui::Text("Cursor: 0x%04X", disasm_state_.cursor_address);
    ImGui::SameLine();
    
    // Run to Cursor button
    if (ImGui::Button("Run to Cursor")) {
        // Add temporary breakpoint at cursor address
        debugger_->add_breakpoint(disasm_state_.cursor_address);
        // Continue execution
        debugger_->continue_execution();
    }
    
    ImGui::Separator();
    
    // Update current address based on follow PC mode
    if (disasm_state_.follow_pc) {
        disasm_state_.current_address = current_pc;
    }
    
    // Scrollable disassembly view
    ImGui::BeginChild("DisassemblyScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // Create disassembler instance
    Disassembler disasm;
    
    // Get breakpoints for display
    const std::vector<Breakpoint>& breakpoints = debugger_->get_breakpoints();
    
    // Calculate address range: 10 lines before PC to 10 lines after PC
    // Each instruction can be 1 or 2 bytes, so we'll estimate
    const int lines_before = 10;
    const int lines_after = 10;
    
    // Calculate start address (go back approximately 10 instructions)
    uint16 start_address = current_pc;
    if (start_address >= lines_before * 2) {
        start_address -= lines_before * 2;  // Assume average 2 bytes per instruction
    } else {
        start_address = 0;
    }
    
    // Disassemble instructions
    uint16 address = start_address;
    int line_count = 0;
    const int max_lines = lines_before + lines_after + 1;
    
    // Track if we should start scrolling to PC
    bool should_scroll_to_pc = false;
    float pc_line_y = 0.0f;
    
    
    while (line_count < max_lines) {
        // Check if this address has a breakpoint
        bool has_breakpoint = false;
        for (const Breakpoint& bp : breakpoints) {
            if (!bp.condition_only && bp.address == address && bp.enabled) {
                has_breakpoint = true;
                break;
            }
        }
        
        // Disassemble instruction at this address
        Instruction instr = disasm.disassemble_instruction(address, memory_ptr);
        
        // Check if this is the current PC instruction
        bool is_current_pc = (address == current_pc);
        
        // Highlight current PC instruction with distinct background color
        if (is_current_pc) {
            // Use PushStyleColor instead of TableSetBgColor since we're not using a table
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));  // Green text
            pc_line_y = ImGui::GetCursorPosY();
            should_scroll_to_pc = disasm_state_.follow_pc;
        }
        
        // Display breakpoint indicator (red dot) for addresses with breakpoints
        if (has_breakpoint) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("\u2022");  // Bullet point (red dot)
            ImGui::PopStyleColor();
        } else {
            ImGui::Text(" ");
        }
        ImGui::SameLine();
        
        // Display address column in hexadecimal format
        ImGui::Text("0x%04X", address);
        ImGui::SameLine();
        
        // Display opcode bytes column in hexadecimal format
        if (instr.has_operand) {
            ImGui::Text("%02X %02X", instr.opcode, instr.operand);
        } else {
            ImGui::Text("%02X   ", instr.opcode);
        }
        ImGui::SameLine();
        
        // Display mnemonic and operands column
        std::string disassembly = disasm.format_instruction(instr);
        
        // Make instruction clickable for cursor positioning
        ImGui::PushID(address);
        bool clicked = ImGui::Selectable(disassembly.c_str(), 
                                        disasm_state_.cursor_address == address,
                                        ImGuiSelectableFlags_SpanAllColumns);
        ImGui::PopID();
        
        // Implement cursor positioning by clicking on instructions
        if (clicked) {
            disasm_state_.cursor_address = address;
            disasm_state_.follow_pc = false;  // Disable follow PC when user clicks
        }
        
        // Pop style color if we pushed it for current PC highlighting
        if (is_current_pc) {
            ImGui::PopStyleColor();
        }
        
        // Move to next instruction
        address += instr.size;
        line_count++;
    }
    
    
    // Auto-scroll to PC if follow PC mode is enabled
    if (should_scroll_to_pc && pc_line_y > 0.0f) {
        // Scroll so PC is roughly in the middle of the view
        float window_height = ImGui::GetWindowHeight();
        float target_scroll = pc_line_y - (window_height / 2.0f);
        if (target_scroll < 0.0f) target_scroll = 0.0f;
        ImGui::SetScrollY(target_scroll);
    }
    
    ImGui::EndChild();
    ImGui::End();
}

void ImGuiDebuggerUI::render_call_stack_panel() {
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Call Stack")) {
        ImGui::End();
        return;
    }
    
    // Get CPU state to access stack
    CPUState cpu_state = emulator_->get_cpu_state();
    
    // Handle empty stack case gracefully
    if (cpu_state.sp == 0) {
        ImGui::Text("Call stack is empty");
        ImGui::End();
        return;
    }
    
    // Get memory state for disassembly
    MemoryState mem_state = emulator_->get_memory_state();
    
    // Determine which memory region to use for disassembly
    const uint8* memory_ptr = nullptr;
    if (!mem_state.cart_rom.empty()) {
        memory_ptr = mem_state.cart_rom.data();
    } else {
        memory_ptr = mem_state.bios_rom;
    }
    
    // Display stack using ImGui::BeginTable()
    if (ImGui::BeginTable("CallStackTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Show columns: Depth, Return Address, Instruction
        ImGui::TableSetupColumn("Depth", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Return Address", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        // Create disassembler instance
        Disassembler disasm;
        
        // Display stack frames (depth 0-7 for Intel 8048)
        // Stack grows upward: sp=0 means empty, sp=1 means one item at stack[0]
        for (int depth = 0; depth < cpu_state.sp && depth < 8; depth++) {
            ImGui::TableNextRow();
            
            // Get return address from stack
            // Intel 8048 uses 12-bit addresses (0x000-0xFFF), mask to 12 bits
            uint16 return_address = cpu_state.stack[depth] & 0x0FFF;
            
            // Column 1: Display depth (0-7 for Intel 8048)
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", depth);
            
            // Column 2: Display return address in hexadecimal format
            ImGui::TableSetColumnIndex(1);
            
            // Make the return address clickable to navigate disassembly
            ImGui::PushID(depth);
            char addr_text[16];
            snprintf(addr_text, sizeof(addr_text), "0x%03X", return_address);
            
            // Implement click handler to navigate disassembly to return address
            if (ImGui::Selectable(addr_text, false, ImGuiSelectableFlags_SpanAllColumns)) {
                // Navigate disassembly viewer to this return address
                disasm_state_.cursor_address = return_address;
                disasm_state_.current_address = return_address;
                disasm_state_.follow_pc = false;  // Disable follow PC when user clicks
            }
            ImGui::PopID();
            
            // Column 3: Disassemble instruction at return address
            ImGui::TableSetColumnIndex(2);
            if (memory_ptr) {
                // Use the disassembly cache for efficiency
                std::string disassembly = disasm_cache_.get_disassembly(return_address, &disasm, memory_ptr);
                ImGui::Text("%s", disassembly.c_str());
            } else {
                ImGui::Text("(no memory loaded)");
            }
        }
        
        ImGui::EndTable();
    }
    
    ImGui::End();
}

void ImGuiDebuggerUI::render_watch_panel() {
    ImGui::SetNextWindowSize(ImVec2(450, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Watch Expressions")) {
        ImGui::End();
        return;
    }
    
    // Get current CPU and memory state for evaluation
    CPUState cpu_state = emulator_->get_cpu_state();
    MemoryState mem_state = emulator_->get_memory_state();
    
    // Add Watch button (disabled if limit reached)
    bool at_limit = watch_state_.expressions.size() >= MAX_WATCH_EXPRESSIONS;
    if (at_limit) {
        ImGui::BeginDisabled();
    }
    
    if (ImGui::Button("Add Watch")) {
        watch_state_.show_error = false;
        watch_state_.error_message.clear();
        ImGui::OpenPopup("Add Watch Expression");
    }
    
    if (at_limit) {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Limit reached (%zu/%zu)", 
                          watch_state_.expressions.size(), MAX_WATCH_EXPRESSIONS);
    } else {
        ImGui::SameLine();
        ImGui::Text("(%zu/%zu)", watch_state_.expressions.size(), MAX_WATCH_EXPRESSIONS);
    }
    
    ImGui::Separator();
    
    // Display watch expression list using table
    if (ImGui::BeginTable("WatchTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        // Setup columns
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Expression", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();
        
        // Display each watch expression
        for (size_t i = 0; i < watch_state_.expressions.size(); i++) {
            WatchExpression& watch = watch_state_.expressions[i];
            
            ImGui::TableNextRow();
            
            // Column 1: Label (display custom label if present)
            ImGui::TableSetColumnIndex(0);
            if (!watch.label.empty()) {
                ImGui::Text("%s", watch.label.c_str());
            } else {
                ImGui::TextDisabled("(no label)");
            }
            
            // Column 2: Expression string
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", watch.expression.c_str());
            
            // Column 3: Evaluate and display current value in hexadecimal
            ImGui::TableSetColumnIndex(2);
            uint16 current_value = watch.evaluate(cpu_state, mem_state);
            
            // Highlight value in distinct color if changed since last step
            if (watch.changed) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));  // Yellow
                ImGui::Text("0x%04X", current_value);
                ImGui::PopStyleColor();
            } else {
                ImGui::Text("0x%04X", current_value);
            }
            
            // Update last_value and changed flag after each evaluation
            if (current_value != watch.last_value) {
                watch.changed = true;
                watch.last_value = current_value;
            } else {
                watch.changed = false;
            }
            
            // Column 4: Delete button
            ImGui::TableSetColumnIndex(3);
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Button("Delete")) {
                // Delete watch expression
                watch_state_.expressions.erase(watch_state_.expressions.begin() + i);
                ImGui::PopID();
                break;  // Exit loop since we modified the vector
            }
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
    
    // Add Watch Expression dialog
    if (ImGui::BeginPopupModal("Add Watch Expression", nullptr, 
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Add a new watch expression:");
        ImGui::Separator();
        
        // Expression input
        ImGui::Text("Expression:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##expression", watch_state_.new_watch_buffer, 
                        sizeof(watch_state_.new_watch_buffer));
        
        ImGui::Spacing();
        ImGui::Text("Examples: 0x1234, A, R0, PC, PSW");
        
        ImGui::Spacing();
        
        // Label input (optional)
        ImGui::Text("Label (optional):");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##label", watch_state_.new_label_buffer, 
                        sizeof(watch_state_.new_label_buffer));
        
        // Display error message if validation failed
        if (watch_state_.show_error) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", watch_state_.error_message.c_str());
        }
        
        ImGui::Separator();
        
        // Add button
        if (ImGui::Button("Add", ImVec2(120, 0))) {
            // Check if expression is empty
            if (watch_state_.new_watch_buffer[0] == '\0') {
                watch_state_.show_error = true;
                watch_state_.error_message = "Expression cannot be empty";
            }
            // Check watch expression limit
            else if (watch_state_.expressions.size() >= MAX_WATCH_EXPRESSIONS) {
                watch_state_.show_error = true;
                watch_state_.error_message = "Watch expression limit reached (max " + std::to_string(MAX_WATCH_EXPRESSIONS) + ")";
            }
            else {
                // Validate expression
                std::string expression_str = watch_state_.new_watch_buffer;
                std::string validation_error;
                
                if (!validate_watch_expression(expression_str, validation_error)) {
                    // Display error message if invalid
                    watch_state_.show_error = true;
                    watch_state_.error_message = validation_error;
                } else {
                    // Validation passed, create new watch expression
                    WatchExpression new_watch;
                    new_watch.expression = expression_str;
                    new_watch.label = std::string(watch_state_.new_label_buffer);
                    new_watch.last_value = 0;
                    new_watch.changed = false;
                    
                    // Determine type based on expression format
                    const char* expr = watch_state_.new_watch_buffer;
                    if ((expr[0] == '0' && (expr[1] == 'x' || expr[1] == 'X')) ||
                        (expr[0] >= '0' && expr[0] <= '9')) {
                        // Looks like a hex address
                        new_watch.type = WatchExpression::Type::MemoryAddress;
                    } else {
                        // Assume it's a register name
                        new_watch.type = WatchExpression::Type::Register;
                    }
                    
                    // Evaluate initial value
                    new_watch.last_value = new_watch.evaluate(cpu_state, mem_state);
                    
                    // Add to watch list
                    watch_state_.expressions.push_back(new_watch);
                    
                    // Clear buffers and close dialog
                    watch_state_.new_watch_buffer[0] = '\0';
                    watch_state_.new_label_buffer[0] = '\0';
                    watch_state_.show_error = false;
                    watch_state_.error_message.clear();
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        
        ImGui::SameLine();
        
        // Cancel button
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            watch_state_.new_watch_buffer[0] = '\0';
            watch_state_.new_label_buffer[0] = '\0';
            watch_state_.show_error = false;
            watch_state_.error_message.clear();
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void ImGuiDebuggerUI::render_controls_panel() {
    ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Controls")) {
        ImGui::End();
        return;
    }
    
    // Get CPU state for displaying PC and instruction count
    CPUState cpu_state = emulator_->get_cpu_state();
    
    // Display current execution status
    ImGui::Text("Status: ");
    ImGui::SameLine();
    if (debugger_->is_paused()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Paused");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Running");
    }
    
    ImGui::Separator();
    
    // Display current PC value
    ImGui::Text("PC: 0x%04X", cpu_state.pc);
    
    // Display instruction count or cycle count
    FrameStats stats = debugger_->get_frame_stats();
    ImGui::Text("Total Cycles: %llu", static_cast<unsigned long long>(stats.total_cycles));
    ImGui::Text("Frame Count: %llu", static_cast<unsigned long long>(stats.frame_count));
    ImGui::Text("FPS: %.2f", stats.fps);
    
    ImGui::Separator();
    
    // Show different buttons based on execution state
    if (debugger_->is_paused()) {
        // When paused, show Step and Continue buttons
        
        // Step (F9) button
        if (ImGui::Button("Step (F9)", ImVec2(150, 0))) {
            debugger_->step();
        }
        
        // Continue (F5) button
        if (ImGui::Button("Continue (F5)", ImVec2(150, 0))) {
            debugger_->continue_execution();
        }
    } else {
        // When running, show Pause button
        
        // Pause button
        if (ImGui::Button("Pause", ImVec2(150, 0))) {
            debugger_->pause();
        }
    }
    
    // Step Over (F8) button
    if (ImGui::Button("Step Over (F8)", ImVec2(150, 0))) {
        // Step over logic: if current instruction is a CALL, set temporary breakpoint after it
        // For now, we'll implement basic step over by checking if next instruction is CALL
        // and stepping until we return to the next instruction
        
        // Get current instruction
        MemoryState mem_state = emulator_->get_memory_state();
        const uint8* memory_ptr = nullptr;
        
        if (!mem_state.cart_rom.empty() && cpu_state.pc < mem_state.cart_rom.size()) {
            memory_ptr = mem_state.cart_rom.data();
        } else if (cpu_state.pc < 1024) {
            memory_ptr = mem_state.bios_rom;
        }
        
        if (memory_ptr) {
            uint8 opcode = memory_ptr[cpu_state.pc];
            
            // Check if current instruction is a CALL (opcodes 0x14, 0x34, 0x54, 0x74, 0x94, 0xB4, 0xD4, 0xF4)
            bool is_call = (opcode & 0x1F) == 0x14;
            
            if (is_call) {
                // Calculate address of next instruction (CALL is 2 bytes)
                uint16 return_address = cpu_state.pc + 2;
                
                // Add temporary breakpoint at return address
                debugger_->add_breakpoint(return_address);
                
                // Continue execution
                debugger_->continue_execution();
                
                // Note: The breakpoint will be hit when we return, but we should remove it
                // This is a simplified implementation - a full implementation would track
                // temporary breakpoints and remove them automatically
            } else {
                // Not a CALL, just step normally
                debugger_->step();
            }
        } else {
            // No memory loaded, just step
            debugger_->step();
        }
    }
    
    // Step Out (Shift+F8) button
    if (ImGui::Button("Step Out (Shift+F8)", ImVec2(150, 0))) {
        // Step out logic: execute until RET instruction completes
        // RET opcodes: 0x83, 0x93 (RET), 0x05, 0x25, 0x45, 0x65, 0x85, 0xA5, 0xC5, 0xE5 (RETR)
        
        // Get current stack depth
        uint8 initial_sp = cpu_state.sp;
        
        // If stack is empty, can't step out
        if (initial_sp == 0) {
            // Already at top level, just step
            debugger_->step();
        } else {
            // Step until stack depth decreases (indicating a return)
            // This is a simplified approach - we'll step and check SP
            // A better implementation would set a breakpoint at the return address
            
            // For now, we'll just step once as a placeholder
            // A full implementation would require tracking stack depth and stepping until return
            debugger_->step();
            
            // TODO: Implement proper step-out by monitoring stack depth
            // This would require stepping in a loop until SP < initial_SP
        }
    }
    
    ImGui::Separator();
    
    // Display Mode combo box
    ImGui::Text("Display Mode:");
    const char* display_mode_items[] = { "Overlay", "Split Screen" };
    int current_mode = (display_mode_ == DisplayMode::Overlay) ? 0 : 1;
    
    if (ImGui::Combo("##DisplayMode", &current_mode, display_mode_items, 2)) {
        // Selection changed, call set_display_mode()
        DisplayMode new_mode = (current_mode == 0) ? DisplayMode::Overlay : DisplayMode::SplitScreen;
        set_display_mode(new_mode);
    }
    
    ImGui::End();
}

// Helper methods (to be implemented in later tasks)
void ImGuiDebuggerUI::render_register(const char* name, uint8 value) {
    (void)name;
    (void)value;
    // To be implemented when needed
}

void ImGuiDebuggerUI::render_register_16(const char* name, uint16 value) {
    (void)name;
    (void)value;
    // To be implemented when needed
}

void ImGuiDebuggerUI::render_psw_flags(uint8 psw) {
    (void)psw;
    // To be implemented when needed
}

void ImGuiDebuggerUI::render_memory_editor(uint16 start_address, size_t size) {
    (void)start_address;
    (void)size;
    // To be implemented when needed
}

void ImGuiDebuggerUI::render_hex_dump(const uint8* data, size_t size, uint16 base_address) {
    (void)data;
    (void)size;
    (void)base_address;
    // To be implemented when needed
}

// Validation methods
bool ImGuiDebuggerUI::validate_breakpoint_condition(const std::string& condition, std::string& error_msg) {
    // Empty condition is valid (address-only breakpoint)
    if (condition.empty()) {
        return true;
    }
    
    // Basic syntax validation for breakpoint conditions
    // Valid patterns: A==0xFF, R0>0x80, PSW&0x10, PC==0x1234
    
    // Check for valid register names
    const char* valid_registers[] = {"A", "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", 
                                     "PC", "PSW", "SP", "P1", "P2", "T"};
    
    // Check if condition contains at least one valid register or memory reference
    bool has_valid_operand = false;
    for (const char* reg : valid_registers) {
        if (condition.find(reg) != std::string::npos) {
            has_valid_operand = true;
            break;
        }
    }
    
    // Check for valid operators: ==, !=, <, >, <=, >=, &, |
    bool has_operator = false;
    const char* valid_operators[] = {"==", "!=", "<=", ">=", "<", ">", "&", "|"};
    for (const char* op : valid_operators) {
        if (condition.find(op) != std::string::npos) {
            has_operator = true;
            break;
        }
    }
    
    // Condition must have at least one operand and one operator
    if (!has_valid_operand) {
        error_msg = "Invalid condition: must reference a valid register (A, R0-R7, PC, PSW, SP, P1, P2, T)";
        return false;
    }
    
    if (!has_operator) {
        error_msg = "Invalid condition: must contain a comparison operator (==, !=, <, >, <=, >=, &, |)";
        return false;
    }
    
    // Check for balanced parentheses
    int paren_count = 0;
    for (char c : condition) {
        if (c == '(') paren_count++;
        if (c == ')') paren_count--;
        if (paren_count < 0) {
            error_msg = "Invalid condition: unbalanced parentheses";
            return false;
        }
    }
    if (paren_count != 0) {
        error_msg = "Invalid condition: unbalanced parentheses";
        return false;
    }
    
    // Basic validation passed
    return true;
}

bool ImGuiDebuggerUI::validate_watch_expression(const std::string& expression, std::string& error_msg) {
    // Empty expression is invalid
    if (expression.empty()) {
        error_msg = "Expression cannot be empty";
        return false;
    }
    
    // Check if it's a valid register name
    const char* valid_registers[] = {"A", "a", "R0", "r0", "R1", "r1", "R2", "r2", 
                                     "R3", "r3", "R4", "r4", "R5", "r5", "R6", "r6", 
                                     "R7", "r7", "PC", "pc", "PSW", "psw", "SP", "sp",
                                     "P1", "p1", "P2", "p2", "PORT1", "port1", 
                                     "PORT2", "port2", "T", "t", "TIMER", "timer"};
    
    for (const char* reg : valid_registers) {
        if (expression == reg) {
            return true;  // Valid register name
        }
    }
    
    // Check if it's a valid memory address (hex format)
    // Valid formats: 0x1234, 0X1234, 1234 (hex)
    const char* expr_str = expression.c_str();
    size_t start_pos = 0;
    
    // Skip optional 0x prefix
    if (expression.length() >= 2 && expr_str[0] == '0' && (expr_str[1] == 'x' || expr_str[1] == 'X')) {
        start_pos = 2;
    }
    
    // Check if remaining characters are valid hex digits
    if (start_pos >= expression.length()) {
        error_msg = "Invalid expression: incomplete hex address";
        return false;
    }
    
    bool all_hex = true;
    for (size_t i = start_pos; i < expression.length(); i++) {
        char c = expr_str[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            all_hex = false;
            break;
        }
    }
    
    if (all_hex) {
        // Valid hex address format
        // Parse to check if it's in valid range
        unsigned int addr;
        if (sscanf(expr_str + start_pos, "%x", &addr) == 1) {
            if (addr > 0xFFFF) {
                error_msg = "Invalid expression: address out of range (must be 0x0000-0xFFFF)";
                return false;
            }
            return true;  // Valid memory address
        }
    }
    
    // Not a valid register or memory address
    error_msg = "Invalid expression: must be a valid register name (A, R0-R7, PC, PSW, SP, P1, P2, T) or hex address (0x0000-0xFFFF)";
    return false;
}

void ImGuiDebuggerUI::handle_shortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    
    // F9: Step
    if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
        debugger_->step();
    }
    
    // F5: Continue
    if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
        debugger_->continue_execution();
    }
    
    // F8: Step Over (without Shift)
    if (ImGui::IsKeyPressed(ImGuiKey_F8) && !io.KeyShift) {
        // Step over logic: if current instruction is a CALL, set temporary breakpoint after it
        CPUState cpu_state = emulator_->get_cpu_state();
        MemoryState mem_state = emulator_->get_memory_state();
        const uint8* memory_ptr = nullptr;
        
        if (!mem_state.cart_rom.empty() && cpu_state.pc < mem_state.cart_rom.size()) {
            memory_ptr = mem_state.cart_rom.data();
        } else if (cpu_state.pc < 1024) {
            memory_ptr = mem_state.bios_rom;
        }
        
        if (memory_ptr) {
            uint8 opcode = memory_ptr[cpu_state.pc];
            
            // Check if current instruction is a CALL (opcodes 0x14, 0x34, 0x54, 0x74, 0x94, 0xB4, 0xD4, 0xF4)
            bool is_call = (opcode & 0x1F) == 0x14;
            
            if (is_call) {
                // Calculate address of next instruction (CALL is 2 bytes)
                uint16 return_address = cpu_state.pc + 2;
                
                // Add temporary breakpoint at return address
                debugger_->add_breakpoint(return_address);
                
                // Continue execution
                debugger_->continue_execution();
            } else {
                // Not a CALL, just step normally
                debugger_->step();
            }
        } else {
            // No memory loaded, just step
            debugger_->step();
        }
    }
    
    // Shift+F8: Step Out
    if (ImGui::IsKeyPressed(ImGuiKey_F8) && io.KeyShift) {
        // Step out logic: execute until RET instruction completes
        CPUState cpu_state = emulator_->get_cpu_state();
        
        // If stack is empty, can't step out
        if (cpu_state.sp == 0) {
            // Already at top level, just step
            debugger_->step();
        } else {
            // Get return address from stack
            uint16 return_address = cpu_state.stack[cpu_state.sp - 1];
            
            // Add temporary breakpoint at return address
            debugger_->add_breakpoint(return_address);
            
            // Continue execution
            debugger_->continue_execution();
        }
    }
    
    // F2: Toggle Breakpoint at current PC or cursor
    if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
        CPUState cpu_state = emulator_->get_cpu_state();
        
        // Use cursor address if in disassembly panel, otherwise use current PC
        uint16 target_address = disasm_state_.follow_pc ? cpu_state.pc : disasm_state_.cursor_address;
        
        // Check if breakpoint already exists at this address
        const std::vector<Breakpoint>& breakpoints = debugger_->get_breakpoints();
        bool found = false;
        
        for (const Breakpoint& bp : breakpoints) {
            if (!bp.condition_only && bp.address == target_address) {
                // Breakpoint exists, remove it
                debugger_->remove_breakpoint(target_address);
                found = true;
                break;
            }
        }
        
        if (!found) {
            // Breakpoint doesn't exist, add it
            debugger_->add_breakpoint(target_address);
        }
    }
    
    // Escape or F12: Close debugger and continue execution
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_F12)) {
        hide();
        debugger_->continue_execution();
    }
}

// WatchExpression evaluation
uint16 WatchExpression::evaluate(const CPUState& cpu, const MemoryState& memory) const {
    if (type == Type::MemoryAddress) {
        // Parse memory address from expression string
        // Expected format: "0x1234" or "1234" (hex)
        uint16 address = 0;
        
        // Try to parse as hexadecimal
        const char* expr_str = expression.c_str();
        if (expr_str[0] == '0' && (expr_str[1] == 'x' || expr_str[1] == 'X')) {
            // Skip "0x" prefix
            expr_str += 2;
        }
        
        // Parse hex digits
        while (*expr_str) {
            char c = *expr_str;
            uint8 digit = 0;
            
            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                digit = 10 + (c - 'a');
            } else if (c >= 'A' && c <= 'F') {
                digit = 10 + (c - 'A');
            } else {
                // Invalid character, return 0
                return 0;
            }
            
            address = (address << 4) | digit;
            expr_str++;
        }
        
        // Read value from memory based on address range
        // CPU internal RAM: 0x0000-0x003F (64 bytes)
        if (address < 0x0040) {
            return cpu.ram[address];
        }
        // External RAM: 0x0040-0x00BF (128 bytes mapped to external_ram[0-127])
        else if (address >= 0x0040 && address < 0x00C0) {
            return memory.external_ram[address - 0x0040];
        }
        // BIOS ROM: 0x0000-0x03FF (1KB) - overlaps with RAM addressing
        // For ROM, we use higher addresses
        else if (address < 0x0400) {
            return memory.bios_rom[address];
        }
        // Cartridge ROM: depends on size
        else if (address < 0x0400 + memory.cart_rom.size()) {
            return memory.cart_rom[address - 0x0400];
        }
        
        return 0;
    }
    else if (type == Type::Register) {
        // Parse register name
        const std::string& reg = expression;
        
        // Accumulator
        if (reg == "A" || reg == "a") {
            return cpu.a;
        }
        // Program Counter
        else if (reg == "PC" || reg == "pc") {
            return cpu.pc;
        }
        // Program Status Word
        else if (reg == "PSW" || reg == "psw") {
            return cpu.psw;
        }
        // Stack Pointer
        else if (reg == "SP" || reg == "sp") {
            return cpu.sp;
        }
        // Working registers R0-R7
        else if (reg.length() == 2 && (reg[0] == 'R' || reg[0] == 'r')) {
            char digit = reg[1];
            if (digit >= '0' && digit <= '7') {
                uint8 reg_num = digit - '0';
                // Use current bank to determine which register to read
                uint8 bank_offset = cpu.current_bank * 8;
                return cpu.r[bank_offset + reg_num];
            }
        }
        // Port 1
        else if (reg == "P1" || reg == "p1" || reg == "PORT1" || reg == "port1") {
            return cpu.port1;
        }
        // Port 2
        else if (reg == "P2" || reg == "p2" || reg == "PORT2" || reg == "port2") {
            return cpu.port2;
        }
        // Timer
        else if (reg == "T" || reg == "t" || reg == "TIMER" || reg == "timer") {
            return cpu.timer;
        }
        
        return 0;
    }
    
    return 0;
}

// DisassemblyCache implementation
std::string DisassemblyCache::get_disassembly(uint16 address, Disassembler* disasm, const uint8* memory) {
    // Check if address is in cache
    auto it = cache.find(address);
    if (it != cache.end()) {
        return it->second;
    }
    
    // Not in cache, disassemble and cache the result
    Instruction instr = disasm->disassemble_instruction(address, memory);
    std::string result = disasm->format_instruction(instr);
    cache[address] = result;
    
    return result;
}

void DisassemblyCache::invalidate() {
    cache.clear();
}
