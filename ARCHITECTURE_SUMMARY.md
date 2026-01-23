# Flight Data Infrastructure - Complete Summary

## Project Structure

```
src/
├── FlightInfo.hpp                    [MODIFIED] - Added comment field
├── LogbookEntry.hpp                  [NEW] - Logbook entry struct
├── LogbookEntry.cpp                  [NEW] - IGC filename generation
└── Logger/
    ├── FlightParser.cpp              [MODIFIED] - Comment parsing support
    ├── LogbookGlue.hpp               [NEW] - Flight enumeration
    ├── LogbookGlue.cpp               [NEW] - Implementation
    ├── LogbookWriter.hpp             [NEW] - Comment writing
    └── LogbookWriter.cpp             [NEW] - Implementation

src/IGC/
└── IGCFileMatcher.hpp                [NEW] - IGC file matching
    IGCFileMatcher.cpp                [NEW] - Implementation
```

## Component Responsibilities

### 1. FlightInfo (Extended)
**Purpose**: Core flight metadata
- Date and times (start/end)
- Comment text
- Duration calculation

### 2. LogbookEntry (New)
**Purpose**: Complete flight record with metadata
- Wraps FlightInfo
- Associates IGC file path
- Provides convenience methods

### 3. FlightParser (Extended)
**Purpose**: Parse flights.log format
- Reads "start", "landing", "comment" records
- Extracts datetime and text
- Maintains line buffering for multi-record sequences

### 4. LogbookWriter (New)
**Purpose**: Append comments to flights.log
- Formats comment records with datetime
- Handles file I/O
- Error handling and recovery

### 5. LogbookGlue (New)
**Purpose**: High-level flight retrieval
- Orchestrates FlightParser
- Sorts flights by date
- Returns complete LogbookEntry objects

### 6. IGCFileMatcher (New)
**Purpose**: Find IGC files matching flights
- Directory scanning with visitor pattern
- IGC header parsing (HFDTE, A records, B records)
- Time-based matching algorithm
- XCSoar file prioritization

## Data Flow Diagram

```
flights.log
    │
    ├─→ FlightParser::Read()
    │   ├─ Parses "start" record → FlightInfo.date, start_time
    │   ├─ Parses "landing" record → FlightInfo.end_time
    │   └─ Parses "comment" record → FlightInfo.comment
    │
    ├─→ LogbookGlue::EnumerateFlights()
    │   ├─ Creates FlightInfo objects
    │   ├─ Wraps in LogbookEntry
    │   └─ Sorts by date descending
    │
    └─→ IGCFileMatcher::FindMatchingIGCFiles()
        ├─ Scans logs/ directory for .igc files
        ├─ Reads HFDTE record for date
        ├─ Reads B-record for start time
        ├─ Matches: start_time - 5min ≤ igc_time ≤ end_time
        └─ Returns: XCSoar files first, then others
```

## File Format Specifications

### flights.log
```
YYYY-MM-DDTHH:MM:SS start
YYYY-MM-DDTHH:MM:SS landing
YYYY-MM-DDTHH:MM:SS comment <any text>
```

### IGC File (HFDTE Record)
```
HFDTE[DD][MM][YY]
Example: HFDTE150124 (15 January 2024)
```

### IGC File (B-Record/Fix)
```
B[HH][MM][SS][DDMM.mmm][N/S][DDDMM.mmm][E/W][validity][pressure_alt][gps_alt][extensions...]
Example: B093045551245N00108200E10103500103450000
```

### IGC File (A Record - Manufacturer)
```
A[MFR][SERIAL][ID][FW][HW][TYPE]
Example: AXCSN123456789ABCDEF
         (XCS = XCSoar manufacturer code)
```

## Key Algorithms

### IGC Time Matching
```
Given: Flight with start_time, end_time
       IGC file with igc_start_time

Match if:
  (flight.start_time - 5 minutes) <= igc_start_time <= flight.end_time
```

### File Sorting
```
Result files:
  1. XCSoar files (A record contains "XCS") - sorted by filename
  2. Other files - sorted by filename
```

### FlightInfo Duration
```
duration = end_time - start_time
(in std::chrono::system_clock::duration)
```

### LogbookEntry IGC Filename
```
Format: YYYYMMDDTHHMM.igc
Example: 20240115T0930.igc (15 Jan 2024, 09:30 UTC)
```

## Error Handling Strategy

All functions marked `noexcept` with graceful degradation:

| Function | Error Condition | Behavior |
|----------|-----------------|----------|
| EnumerateFlights() | Missing flights.log | Returns empty vector |
| EnumerateFlights() | Read error | Returns empty vector |
| FindMatchingIGCFiles() | Invalid IGC | Skips silently |
| FindMatchingIGCFiles() | Read error | Returns empty vector |
| WriteComment() | File write error | Returns false |
| IGCFileMatcher::IGCFileMatches() | Parse error | Returns false |

## Backward Compatibility

✓ Existing flights.log files without comments work unchanged
✓ FlightParser handles logs predating comment support
✓ LogbookWriter can append to existing logs
✓ FlightInfo remains compatible (comment field is initially empty)
✓ No breaking changes to existing APIs

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| EnumerateFlights() | O(n) | n = number of flight records |
| FindMatchingIGCFiles() | O(m log m) | m = number of IGC files (sorting) |
| IGC file read | O(k) | k = number of lines until first B-record |
| Comment write | O(1) | Single append operation |
| IGC filename gen | O(1) | Fixed-size formatting |

## Testing Coverage

Essential test cases:

1. **FlightParser**
   - Parse start record
   - Parse landing record
   - Parse comment record
   - Handle missing datetime
   - Backward compat (no comments)

2. **LogbookEntry**
   - GetDuration() calculation
   - GetIGCFilename() formatting
   - HasIGCFile() detection

3. **LogbookWriter**
   - Append to new file
   - Append to existing file
   - Handle write errors
   - Format validation

4. **LogbookGlue**
   - Read all flights
   - Sort order verification
   - Comment association
   - Error handling

5. **IGCFileMatcher**
   - Match flights within window
   - 5-minute tolerance
   - XCSoar detection
   - Filename sorting
   - Skip invalid files

## Maintenance Notes

- Comment parsing uses simple string comparison for "comment" keyword
- IGC "A" record detection assumes "XCS" in positions 1-3
- Time matching uses simple datetime comparison
- No caching of IGC file metadata (scans directory each time)
- File I/O uses standard XCSoar stream classes
