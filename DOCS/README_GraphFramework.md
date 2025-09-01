# Mining Stats Graph Framework

## Overview
The Mining Stats screen provides a cycleable graph system integrated directly into `miningStatsScreen.cpp`. It allows users to cycle through different graph types by clicking on the graph area.

## Features
- **Cycleable Graphs**: Click anywhere on the graph to cycle between different graph types
- **Modern Container Sizing**: Uses 632x352 container size with optimized chart dimensions (600x250)
- **3 Built-in Graph Types**:
  - **Hashrate Graph**: Shows mining hashrate over time with pool info, difficulty, and reject rate
  - **Power Graph**: Displays power consumption with voltage and efficiency details  
  - **Temperature Graph**: Shows ASIC temperature with fan speed and VREG temperature

## Implementation Details

### Container Architecture
- **Container Size**: 632x352 (modern sizing pattern)
- **Chart Size**: 600x250 (optimized for readability)
- **Padding**: 16px margins for clean layout
- **Footer/Header**: 8px spacing for labels

### Graph Structure
Each graph follows a consistent layout:
- **Top Labels**: Title on left, current value on right
- **Center**: Chart area with transparent background and bottom border
- **Bottom Labels**: Additional metrics aligned left, center, and right

### Sizing Adjustments Made
From the original implementation:
- Container: 672x312 → 632x352 (modern style)
- Chart: 648x280 → 600x250 (better proportions)
- Alignment: Adjusted for new container size
- Padding: Standardized 16px margins

## Usage

The framework is automatically initialized when the mining stats screen loads:

```cpp
void miningStatsScreen() {
    // Graph framework is integrated directly
    // Creates 3 containers with modern sizing
    // Sets up hashrate, power, and temperature graphs
    // Initializes with hashrate graph visible
}
```

## User Interaction

1. **Navigate** to the mining tab
2. **Click anywhere** on the graph area to cycle between:
   - Hashrate Graph (shows first)
   - Power Graph
   - Temperature Graph
3. **View real-time data** with 2-second update intervals

## Performance Optimizations

- **Timer Management**: Only active graph updates (others paused)
- **LVGL Locking**: Proper synchronization for thread safety
- **Memory Efficient**: Static containers reduce allocation overhead
- **Auto-ranging**: Hashrate graph automatically adjusts Y-axis scale

## Architecture Benefits

- **Integrated Design**: No separate framework files needed
- **Modern Sizing**: Consistent with current UI patterns
- **Optimized Performance**: Minimal resource usage
- **Easy Maintenance**: All code in one location
- **Theme Support**: Automatically adapts to current theme colors

## Code Structure

```cpp
// State management
static MiningGraphContainerState miningGraphState;

// Container creation
static void createMiningGraphContainer(lv_obj_t* parent, lv_obj_t*& container);

// Graph implementations
static void createHashrateGraph(lv_obj_t* parent);
static void createPowerGraph(lv_obj_t* parent);
static void createTemperatureGraph(lv_obj_t* parent);

// Visibility control
static void updateMiningGraphVisibility(int graphIndex);
static void miningGraphContainerEventCallback(lv_event_t* e);
```

This integrated approach provides the same great user experience while being more maintainable and following modern UI sizing conventions. 

# Graph Container Framework

## Overview
The Graph Container Framework provides a standardized way to create cycleable graph containers similar to the quadContainer system. It consists of two main functions that work together to create consistent, themed graph displays.

## Framework Functions

### `graphContainer()`
Creates a single graph container with proper styling and dimensions.

```cpp
void graphContainer(lv_obj_t* parentMainContainer, lv_obj_t*& container);
```

**Parameters:**
- `parentMainContainer`: The parent container to attach the graph container to
- `container`: Reference to the container object that will be created

**Features:**
- Modern container sizing (632x352)
- Transparent background with themed border
- Clickable for user interaction
- Non-scrollable by default

### `graphContainerContent()`
Adds standardized content to a graph container including title, value, chart, and footer labels.

```cpp
void graphContainerContent(lv_obj_t* container, const char* title, const char* currentValue, const char* bottomLeftLabel, const char* bottomRightLabel);
```

**Parameters:**
- `container`: The container to add content to (created by `graphContainer()`)
- `title`: Main title displayed in top-left (e.g., "Hashrate", "Power", "Temperature")
- `currentValue`: Current value displayed next to title (e.g., "350.5 GH/s", "45.2 W")
- `bottomLeftLabel`: Left footer information (e.g., "Pool: stratum+tcp://...")
- `bottomRightLabel`: Right footer information (e.g., "Efficiency: 45 W/TH")

**Generated Components:**
- **Title Label**: Top-left, 24px medium font
- **Value Label**: Top-right of title, 24px medium font  
- **Chart**: Center area, 600x250px with themed styling
- **Bottom Labels**: Footer information, 16px medium font

## Layout Structure

```
┌─────────────────────────────────────────────────────────────┐
│ Title                    Current Value                      │
│                                                             │
│                                                             │
│                     Chart Area                              │
│                    (600x250)                               │
│                                                             │
│                                                             │
│ Bottom Left Info              Bottom Right Info             │
└─────────────────────────────────────────────────────────────┘
```

## Usage Example

### Basic Usage
```cpp
// Create a single graph container
lv_obj_t* myGraphContainer;
graphContainer(screenObjs.miningMainContainer, myGraphContainer);

// Add content to the container
graphContainerContent(
    myGraphContainer,
    "Hashrate",                           // title
    "350.5 GH/s",                        // current value
    "Pool: stratum+tcp://pool.com:4334", // bottom left
    "Difficulty: 1.2M"                   // bottom right
);
```

### Integration with Cycleable System
```cpp
// State management structure (defined in header)
struct MiningGraphContainerState {
    lv_obj_t* containers[MINING_GRAPH_MAX_VARIATIONS];
    lv_timer_t* timers[MINING_GRAPH_MAX_VARIATIONS];
    int currentIndex;
};

// Create multiple graph containers
MiningGraphContainerState graphState;

// Initialize containers
for (int i = 0; i < 3; i++) {
    graphContainer(screenObjs.miningMainContainer, graphState.containers[i]);
}

// Add different content to each
graphContainerContent(graphState.containers[0], 
    "Hashrate", "350.5 GH/s", 
    "Pool: stratum+tcp://pool.com:4334", 
    "Reject Rate: 2.1%");

graphContainerContent(graphState.containers[1], 
    "Power", "45.2 W", 
    "Voltage: 12.050 V", 
    "Efficiency: 45 W/TH");

graphContainerContent(graphState.containers[2], 
    "Temperature", "65°C", 
    "Fan: 2840 RPM (75%)", 
    "VREG: 78°C");
```

## Chart Data Management

The framework creates charts with these default settings:
- **Type**: Line chart
- **Points**: 50 data points
- **Styling**: 4px line width, themed colors
- **Series**: Single series with primary theme color

### Adding Data to Charts
```cpp
// Get the chart object (3rd child of container after title and value labels)
lv_obj_t* chart = lv_obj_get_child(container, 2);
lv_chart_series_t* series = lv_chart_get_series_next(chart, NULL);

// Add data point
lv_chart_set_next_value(chart, series, newValue);
```

### Chart Customization
```cpp
// Auto-ranging example
lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, minValue, maxValue);

// Update chart with timer
lv_timer_create([](lv_timer_t* timer) {
    lv_obj_t* chart = (lv_obj_t*)timer->user_data;
    // Update chart data
    lv_chart_set_next_value(chart, series, getCurrentValue());
}, 2000, chart);
```

## Event Handling

### Container Click Events
The framework makes containers clickable by default. Add event handlers for cycling:

```cpp
static void graphContainerEventCallback(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    // Handle container click for cycling
    cycleToNextGraph();
}

// Add event callback after creating container
lv_obj_add_event_cb(container, graphContainerEventCallback, LV_EVENT_CLICKED, NULL);
```

## Comparison to QuadContainer

| Feature | QuadContainer | GraphContainer |
|---------|---------------|----------------|
| **Purpose** | 4 small info containers | Single large graph container |
| **Size** | 295x165 each | 632x352 single |
| **Content** | Title + Value | Title + Value + Chart + 2 Footer Labels |
| **Layout** | 2x2 grid | Single centered |
| **Use Case** | Status displays | Time-series data visualization |

## Migration from Custom Implementation

### Before (Custom)
```cpp
// Custom container creation with manual styling
lv_obj_t* container = lv_obj_create(parent);
lv_obj_set_size(container, 632, 352);
// ... 20+ lines of styling code ...

// Custom content creation
lv_obj_t* title = lv_label_create(container);
// ... 10+ lines per label ...
lv_obj_t* chart = lv_chart_create(container);
// ... 15+ lines of chart setup ...
```

### After (Framework)
```cpp
// Framework approach - 2 function calls
lv_obj_t* container;
graphContainer(parent, container);
graphContainerContent(container, "Title", "Value", "Left Info", "Right Info");
```

## Benefits

1. **Consistency**: All graph containers follow the same layout and styling
2. **Maintainability**: Centralized styling that updates with theme changes
3. **Simplicity**: Complex graph setup reduced to 2 function calls  
4. **Flexibility**: Easy to create multiple graphs with different data
5. **Theme Support**: Automatic adaptation to current theme colors
6. **Reusability**: Can be used across different screens and contexts

## Future Enhancements

- **Multiple Chart Types**: Support for bar charts, area charts, etc.
- **Custom Layouts**: Configurable label positions
- **Animation Support**: Smooth transitions between graphs
- **Data Binding**: Automatic data source integration
- **Export Functions**: Screenshot/data export capabilities

This framework provides a solid foundation for consistent graph displays while maintaining the flexibility to customize individual implementations as needed. 