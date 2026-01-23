# Quick Reference - Flight Data Infrastructure

## Files at a Glance

| File | Type | Lines | Purpose |
|------|------|-------|---------|
| src/FlightInfo.hpp | Modified | 18 | Added `std::string comment` field |
| src/LogbookEntry.hpp | New | 48 | Struct combining flight + IGC + comment |
| src/LogbookEntry.cpp | New | 15 | GetIGCFilename() implementation |
| src/Logger/FlightParser.cpp | Modified | +3 | Parse comment records |
| src/Logger/LogbookWriter.hpp | New | 28 | Write comments to flights.log |
| src/Logger/LogbookWriter.cpp | New | 38 | Implementation |
| src/Logger/LogbookGlue.hpp | New | 22 | Enumerate flights function |
| src/Logger/LogbookGlue.cpp | New | 46 | Implementation |
| src/IGC/IGCFileMatcher.hpp | New | 30 | Find matching IGC files |
| src/IGC/IGCFileMatcher.cpp | New | 137 | Implementation |

## API Quick Reference

### Read Flights
```cpp
#include "Logger/LogbookGlue.hpp"

auto flights = EnumerateFlights();
for (const auto& entry : flights) {
  // entry.flight.date
  // entry.flight.start_time
  // entry.flight.end_time
  // entry.flight.comment
  // entry.igc_file
  // entry.GetDuration()
  // entry.GetIGCFilename()
  // entry.HasIGCFile()
}
```

### Write Comment
```cpp
#include "Logger/LogbookWriter.hpp"

BrokenDateTime start(2024, 1, 15, 9, 30, 0);
bool ok = LogbookWriter::WriteComment(
  LocalPath(_T("flights.log")),
  start,
  "Great flight!"
);
```

### Find IGC Files
```cpp
#include "IGC/IGCFileMatcher.hpp"

auto igc_files = FindMatchingIGCFiles(flight);
// Returns vector of AllocatedPath
// XCSoar files first, then others by filename
```

## Data Formats

### flights.log
```
2024-01-15T09:30:00 start
2024-01-15T11:45:30 landing
2024-01-15T09:30:00 comment Text goes here
```

### LogbookEntry Fields
```
flight.date          // BrokenDate
flight.start_time    // BrokenTime
flight.end_time      // BrokenTime
flight.comment       // std::string
igc_file             // AllocatedPath
```

### IGC Filename Format
```
YYYYMMDDTHHMM.igc
Example: 20240115T0930.igc
```

## Key Features

✓ **Comment Support**: Parse and write flight comments
✓ **IGC Matching**: Time-based with 5-minute tolerance
✓ **XCSoar Priority**: XCSoar files returned first
✓ **Backward Compatible**: Works with existing logs
✓ **Error Handling**: All functions `noexcept` with graceful degradation
✓ **Sorting**: Flights by date (descending), IGC files by type+name

## Time Matching Logic

```
Match if:
  (flight.start_time - 5 min) <= igc_start_time <= flight.end_time
  
Detected from:
  - HFDTE record: IGC date
  - B record: IGC start time
  - A record: "XCS" = XCSoar file
```

## Dependencies

### New Headers
- `#include "time/BrokenDateTime.hpp"`
- `#include "fs/AllocatedPath.hpp"`
- `#include "IGC/IGCParser.hpp"`
- `#include "io/FileOutputStream.hxx"`
- `#include "io/FileLineReaderA.hpp"`
- `#include "LocalPath.hpp"`

### Used Classes
- `FileOutputStream` - Write files
- `FileLineReaderA` - Read text files
- `IGCParseDateRecord()` - Parse HFDTE
- `IGCParseFix()` - Parse B records
- `VisitDataFiles()` - Directory traversal
- `File::Visitor` - File visitor pattern
- `AllocatedPath` - File path handling

## Build Integration

Add to build system:
```make
src/LogbookEntry.cpp
src/Logger/LogbookWriter.cpp
src/Logger/LogbookGlue.cpp
src/IGC/IGCFileMatcher.cpp
```

## Common Patterns

### Iterate Flights
```cpp
for (auto& entry : EnumerateFlights()) {
  // Use entry...
}
```

### Create LogbookEntry
```cpp
LogbookEntry entry;
entry.flight = flight_info;
entry.comment = "Comment text";
entry.igc_file = AllocatedPath::Null();
```

### Check Flight Properties
```cpp
auto duration = entry.GetDuration();
auto filename = entry.GetIGCFilename();
bool has_igc = entry.HasIGCFile();
```

## Error Handling

All functions return empty/false on error:
- `EnumerateFlights()` → empty vector
- `FindMatchingIGCFiles()` → empty vector
- `WriteComment()` → false
- No exceptions thrown

## Performance Notes

- **Enumeration**: O(n) where n = flight records
- **IGC Matching**: O(m log m) where m = IGC files
- **Memory**: All flights loaded (suitable for ~10k flights)
- **File I/O**: Standard sequential reads

## Debugging Tips

1. Check flights.log exists in data directory
2. Verify comment format: `YYYY-MM-DDTHH:MM:SS comment <text>`
3. Ensure IGC files have HFDTE and B records
4. Check XCSoar files have "XCS" in A record
5. Use 5-minute tolerance window for IGC matching

## Documentation Files

- `FLIGHT_INFRASTRUCTURE.md` - Architecture overview
- `IMPLEMENTATION_GUIDE.md` - Integration guide
- `ARCHITECTURE_SUMMARY.md` - Technical details
- `IMPLEMENTATION_CHECKLIST.md` - Completion status
