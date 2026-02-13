// WARNING: Never insert or delete items or rearrange the order of the items
// in this list. This will break existing infobox configurations of all users!

// e_HeightGPS
INFOBOX_ENTRY(e_HeightGPS,
              N_("Altitude GPS"),
              N_("Alt GPS"),
              N_("Altitude above mean sea level reported by the GPS. "
                 "(Touch-screen/PC only) In simulation mode, this value is "
                 "adjustable with the up/down arrow keys; the right/left "
                 "arrow keys cause the glider to turn."),
              IBFHelper<InfoBoxContentAltitudeGPS>::Create,
              nullptr,
              nullptr)

// e_HeightAGL
INFOBOX_ENTRY(e_HeightAGL,
              N_("Height AGL"),
              N_("H AGL"),
              N_("Navigation altitude minus the terrain elevation obtained "
                 "from the terrain file. The value is coloured red when the "
                 "glider is below the terrain safety clearance height."),
              nullptr,
              UpdateInfoBoxAltitudeAGL,
              altitude_infobox_panels)

// e_Thermal_30s
INFOBOX_ENTRY(e_Thermal_30s,
              N_("Thermal climb, last 30 s"),
              N_("TC 30s"),
              N_("30-second rolling average climb rate based on reported GPS "
                 "altitude, or vario if available. The number in smaller "
                 "font reflects the climb rate for the current thermal since "
                 "circling started."),
              nullptr,
              UpdateInfoBoxThermal30s,
              nullptr)

// e_Bearing
INFOBOX_ENTRY(e_Bearing,
              N_("Next bearing"),
              N_("Bearing"),
              N_("True bearing of the next waypoint. For AAT tasks, this is "
                 "the true bearing to the target within the AAT sector."),
              IBFHelper<InfoBoxContentBearing>::Create,
              nullptr,
              nullptr)

// e_GR_Instantaneous
INFOBOX_ENTRY(e_GR_Instantaneous,
              N_("GR instantaneous"),
              N_("GR Inst"),
              N_("Instantaneous glide ratio over ground, given by the ground "
                 "speed divided by the vertical speed (GPS speed) over the "
                 "last 20 seconds. Negative values indicate climbing cruise. "
                 "If the vertical speed is close to zero, the displayed value "
                 "is '---'."),
              nullptr,
              UpdateInfoBoxGRInstant,
              nullptr)

// e_GR_Cruise
INFOBOX_ENTRY(e_GR_Cruise,
              N_("GR cruise"),
              N_("GR Cruise"),
              N_("Distance from the top of the last thermal, divided by the "
                 "altitude lost since the top of the last thermal. Negative "
                 "values indicate climbing cruise (height gain since leaving "
                 "the last thermal). If the vertical speed is close to zero, "
                 "the displayed value is '---'."),
              nullptr,
              UpdateInfoBoxGRCruise,
              nullptr)

// e_Speed_GPS
INFOBOX_ENTRY(e_Speed_GPS,
              N_("Speed ground"),
              N_("V GND"),
              N_("Ground speed measured by the GPS. The small value shows the "
                 "head or tail wind difference to TAS for the current "
                 "vector. If this InfoBox is active in simulation mode, "
                 "pressing the up and down arrows adjusts the speed, and left "
                 "and right turn the glider."),
              IBFHelper<InfoBoxContentSpeedGround>::Create,
              nullptr,
              nullptr)

// e_TL_Avg
INFOBOX_ENTRY(e_TL_Avg,
              N_("Last thermal average"),
              N_("TL Avg"),
              N_("Total altitude gain/loss in the last thermal divided by "
                 "the time spent circling."),
              nullptr,
              UpdateInfoBoxThermalLastAvg,
              nullptr)

// e_TL_Gain
INFOBOX_ENTRY(e_TL_Gain,
              N_("Last thermal gain"),
              N_("TL Gain"),
              N_("Total altitude gain/loss in the last thermal. The number in "
                 "smaller font reflects the overall climb rate for the last "
                 "thermal."),
              nullptr,
              UpdateInfoBoxThermalLastGain,
              nullptr)

// e_TL_Time
INFOBOX_ENTRY(e_TL_Time,
              N_("Last thermal duration"),
              N_("TL duration"),
              N_("Time spent circling in the last thermal."),
              nullptr,
              UpdateInfoBoxThermalLastTime,
              nullptr)

// e_MacCready
INFOBOX_ENTRY(e_MacCready,
              N_("MacCready setting"),
              N_("MC"),
              N_("Current MacCready setting and mode (manual or auto). "
                 "(Touch-screen/PC only) When this InfoBox is active, use the "
                 "up/down cursor keys to adjust the MacCready setting."),
              IBFHelper<InfoBoxContentMacCready>::Create,
              nullptr,
              nullptr)

// e_WP_Distance
INFOBOX_ENTRY(e_WP_Distance,
              N_("Next distance"),
              N_("WP Dist"),
              N_("Distance to the currently selected waypoint. For AAT tasks, "
                 "this is the distance to the target within the AAT sector."),
              IBFHelper<InfoBoxContentNextDistance>::Create,
              nullptr,
              nullptr)

// e_WP_AltDiff
INFOBOX_ENTRY(e_WP_AltDiff,
              N_("Next altitude difference"),
              N_("WP AltD"),
              N_("Arrival altitude at the next waypoint relative to the "
                 "safety arrival height. For AAT tasks, the target within the "
                 "AAT sector is used."),
              IBFHelper<InfoBoxContentNextAltitudeDiff>::Create,
              nullptr,
              nullptr)

// e_WP_AltReq
INFOBOX_ENTRY(e_WP_AltReq,
              N_("Next altitude required"),
              N_("WP AltR"),
              N_("Additional altitude required to reach the next turn point. "
                 "For AAT tasks, the target within the AAT sector is used."),
              IBFHelper<InfoBoxContentNextAltitudeRequire>::Create,
              nullptr,
              nullptr)

// e_WP_Name
INFOBOX_ENTRY(e_WP_Name,
              N_("Next waypoint"),
              N_("Next WP"),
              N_("Name of the currently selected turn point. When this "
                 "InfoBox is active, using the up/down cursor keys selects "
                 "the next/previous waypoint in the task. (Touch-screen/PC "
                 "only) Pressing the enter cursor key brings up the "
                 "Airfields/waypoint details."),
              IBFHelper<InfoBoxContentNextWaypoint>::Create,
              nullptr,
              nullptr)

// e_Fin_AltDiff
INFOBOX_ENTRY(e_Fin_AltDiff,
              N_("Final altitude difference"),
              N_("Fin AltD"),
              N_("Arrival altitude at the final task turn point relative to "
                 "the safety arrival height."),
              nullptr,
              UpdateInfoBoxFinalAltitudeDiff,
              nullptr)

// e_Fin_AltReq
INFOBOX_ENTRY(e_Fin_AltReq,
              N_("Final altitude required"),
              N_("Fin AltR"),
              N_("Additional altitude required to finish the task."),
              nullptr,
              UpdateInfoBoxFinalAltitudeRequire,
              nullptr)

// e_SpeedTaskAvg
INFOBOX_ENTRY(e_SpeedTaskAvg,
              N_("Speed task average"),
              N_("V Task Avg"),
              N_("Average cross-country speed while on current task, not "
                 "compensated for altitude."),
              nullptr,
              UpdateInfoBoxTaskSpeed,
              nullptr)

// e_Fin_Distance
INFOBOX_ENTRY(e_Fin_Distance,
              N_("Final distance"),
              N_("Fin Dist"),
              N_("Distance to finish around remaining turn points."),
              nullptr,
              UpdateInfoBoxFinalDistance,
              nullptr)

// e_Fin_GR_TE
INFOBOX_ENTRY(e_Fin_GR_TE,
              _T("Final GR (TE) deprecated"),
              _T("---"),
              _T("Deprecated, there is no TE compensation on GR, you should "
                 "switch to the \"Final GR\" info box."),
              nullptr,
              UpdateInfoBoxFinalGR,
              nullptr)

// e_H_Terrain
INFOBOX_ENTRY(e_H_Terrain,
              N_("Terrain elevation"),
              N_("Terr Elev"),
              N_("Elevation of the terrain above mean sea level, obtained "
                 "from the terrain file at the current GPS location."),
              nullptr,
              UpdateInfoBoxTerrainHeight,
              nullptr)

// e_Thermal_Avg
INFOBOX_ENTRY(e_Thermal_Avg,
              N_("Thermal average"),
              N_("TC Avg"),
              N_("Altitude gained/lost in the current thermal, divided by "
                 "time spent thermalling."),
              nullptr,
              UpdateInfoBoxThermalAvg,
              nullptr)

// e_Thermal_Gain
INFOBOX_ENTRY(e_Thermal_Gain,
              N_("Thermal gain"),
              N_("TC Gain"),
              N_("Altitude gained/lost in the current thermal."),
              nullptr,
              UpdateInfoBoxThermalGain,
              nullptr)

// e_Track_GPS
INFOBOX_ENTRY(e_Track_GPS,
              N_("Track"),
              N_("Track"),
              N_("Magnetic track reported by the GPS. (Touch-screen/PC only) "
                 "If this InfoBox is active in simulation mode, pressing the "
                 "up/down arrows adjusts the track."),
              IBFHelper<InfoBoxContentTrack>::Create,
              nullptr,
              nullptr)

// e_VerticalSpeed_GPS
INFOBOX_ENTRY(e_VerticalSpeed_GPS,
              N_("Vario"),
              N_("Vario"),
              N_("Instantaneous vertical speed, as reported by the GPS, or "
                 "the intelligent vario total energy vario value if connected "
                 "to one."),
              nullptr,
              UpdateInfoBoxVario,
              nullptr)

// e_WindSpeed_Est
INFOBOX_ENTRY(e_WindSpeed_Est,
              N_("Wind speed"),
              N_("Wind"),
              N_("Wind speed estimated by XCSoar. Manual adjustment is "
                 "possible with the connected InfoBox dialogue. Pressing the "
                 "up/down cursor keys to cycle through settings, adjust the "
                 "values with left/right cursor keys."),
              IBFHelper<InfoBoxContentWindSpeed>::Create,
              nullptr,
              nullptr)

// e_WindBearing_Est
INFOBOX_ENTRY(e_WindBearing_Est,
              N_("Wind bearing"),
              N_("Wind"),
              N_("Wind bearing estimated by XCSoar. Manual adjustment is "
                 "possible with the connected InfoBox dialogue. Pressing the "
                 "up/down cursor keys to cycle through settings, adjust the "
                 "values with left/right cursor keys."),
              IBFHelper<InfoBoxContentWindBearing>::Create,
              nullptr,
              nullptr)

// e_AA_Time
INFOBOX_ENTRY(e_AA_Time,
              N_("AAT time"),
              N_("AAT Time"),
              N_("Assigned Area Task time remaining. Goes red when time "
                 "remaining has expired."),
              nullptr,
              UpdateInfoBoxTaskAATime,
              nullptr)

// e_AA_DistanceMax
INFOBOX_ENTRY(e_AA_DistanceMax,
              N_("AAT max. distance "),
              N_("AAT Dmax"),
              N_("Assigned Area Task maximum distance possible for remainder "
                 "of task."),
              nullptr,
              UpdateInfoBoxTaskAADistanceMax,
              nullptr)

// e_AA_DistanceMin
INFOBOX_ENTRY(e_AA_DistanceMin,
              N_("AAT min. distance"),
              N_("AAT Dmin"),
              N_("Assigned Area Task minimum distance possible for remainder "
                 "of task."),
              nullptr,
              UpdateInfoBoxTaskAADistanceMin,
              nullptr)

// e_AA_SpeedMax
INFOBOX_ENTRY(e_AA_SpeedMax,
              N_("AAT speed max. distance"),
              N_("AAT Vmax"),
              N_("Assigned Area Task average speed achievable if flying "
                 "maximum possible distance remaining in minimum AAT time."),
              nullptr,
              UpdateInfoBoxTaskAASpeedMax,
              nullptr)

// e_AA_SpeedMin
INFOBOX_ENTRY(e_AA_SpeedMin,
              N_("AAT speed min. distance"),
              N_("AAT Vmin"),
              N_("Assigned Area Task average speed achievable if flying "
                 "minimum possible distance remaining in minimum AAT time."),
              nullptr,
              UpdateInfoBoxTaskAASpeedMin,
              nullptr)

// e_AirSpeed_Ext
INFOBOX_ENTRY(e_AirSpeed_Ext,
              N_("Airspeed IAS"),
              N_("V IAS"),
              N_("Indicated Airspeed reported by a supported external "
                 "intelligent vario."),
              nullptr,
              UpdateInfoBoxSpeedIndicated,
              nullptr)

// e_H_Baro
INFOBOX_ENTRY(e_H_Baro,
              N_("Barometric altitude"),
              N_("Alt Baro"),
              N_("Barometric altitude obtained from a device equipped with a "
                 "pressure sensor."),
              nullptr,
              UpdateInfoBoxAltitudeBaro,
              altitude_infobox_panels)

// e_WP_Speed_MC
INFOBOX_ENTRY(e_WP_Speed_MC,
              N_("Speed MacCready"),
              N_("V MC"),
              N_("MacCready speed-to-fly for optimal flight to the next "
                 "waypoint. In cruise flight mode, this speed-to-fly is "
                 "calculated for maintaining altitude. In final glide mode, "
                 "this speed-to-fly is calculated for descent."),
              nullptr,
              UpdateInfoBoxSpeedMacCready,
              nullptr)

// e_Climb_Perc
INFOBOX_ENTRY(e_Climb_Perc,
              N_("Percentage climb"),
              N_("% Climb"),
              N_("Percentage of time spent in climb mode. These statistics "
                 "are reset upon starting the task."),
              nullptr,
              UpdateInfoBoxThermalRatio,
              nullptr)

// e_TimeSinceTakeoff
INFOBOX_ENTRY(e_TimeSinceTakeoff,
              N_("Flight duration"),
              N_("Flt Duration"),
              N_("Time elapsed since takeoff was detected."),
              nullptr,
              UpdateInfoBoxTimeFlight,
              nullptr)

// e_Load_G
INFOBOX_ENTRY(e_Load_G,
              N_("G load"),
              N_("G"),
              N_("Magnitude of G loading reported by a supported external "
                 "intelligent vario. This value is negative for pitch-down "
                 "manoeuvres."),
              nullptr,
              UpdateInfoBoxGLoad,
              nullptr)

// e_WP_GR
INFOBOX_ENTRY(e_WP_GR,
              N_("Next GR"),
              N_("WP GR"),
              N_("Required glide ratio over ground to reach the next "
                 "waypoint, given by the distance to the next waypoint "
                 "divided by the height required to arrive at the safety "
                 "arrival height."),
              IBFHelper<InfoBoxContentNextGR>::Create,
              nullptr,
              nullptr)

// e_TimeLocal
INFOBOX_ENTRY(e_TimeLocal,
              N_("Time local"),
              N_("Time loc"),
              N_("GPS time expressed in local time zone."),
              nullptr,
              UpdateInfoBoxTimeLocal,
              nullptr)

// e_TimeUTC
INFOBOX_ENTRY(e_TimeUTC,
              N_("Time UTC"),
              N_("Time UTC"),
              N_("GPS time expressed in UTC."),
              nullptr,
              UpdateInfoBoxTimeUTC,
              nullptr)

// e_Fin_Time
INFOBOX_ENTRY(e_Fin_Time,
              N_("Task time to go"),
              N_("Fin ETE"),
              N_("Estimated time required to complete task, assuming "
                 "performance of ideal MacCready cruise/climb cycle."),
              nullptr,
              UpdateInfoBoxFinalETE,
              nullptr)

// e_WP_Time
INFOBOX_ENTRY(e_WP_Time,
              N_("Next time to go"),
              N_("WP ETE"),
              N_("Estimated time required to reach next waypoint, assuming "
                 "performance of ideal MacCready cruise/climb cycle."),
              IBFHelper<InfoBoxContentNextETE>::Create,
              nullptr,
              nullptr)

// e_Act_Speed
INFOBOX_ENTRY(e_Act_Speed,
              N_("Speed dolphin"),
              N_("Vopt"),
              N_("Instantaneous MacCready speed-to-fly, making use of netto "
                 "vario calculations to determine dolphin cruise speed on the "
                 "glider's current track. In cruise flight mode, this "
                 "speed-to-fly is calculated for maintaining altitude. In "
                 "final glide mode, this speed-to-fly is calculated for "
                 "descent. In climb mode, this switches to the speed for "
                 "minimum sink at the current load factor (if an "
                 "accelerometer is connected). When Block mode speed-to-fly "
                 "is selected, this InfoBox displays the MacCready speed."),
              nullptr,
              UpdateInfoBoxSpeedDolphin,
              nullptr)

// e_VerticalSpeed_Netto
INFOBOX_ENTRY(e_VerticalSpeed_Netto,
              N_("Netto vario"),
              N_("Netto"),
              N_("Instantaneous vertical speed of air-mass, equal to vario "
                 "value less the glider's estimated sink rate. Best used if "
                 "airspeed, accelerometers and vario are connected, otherwise "
                 "calculations are based on GPS measurements and wind "
                 "estimates."),
              nullptr,
              UpdateInfoBoxVarioNetto,
              nullptr)

// e_Fin_TimeLocal
INFOBOX_ENTRY(e_Fin_TimeLocal,
              N_("Task arrival time"),
              N_("Fin ETA"),
              N_("Estimated arrival local time at task completion, assuming "
                 "performance of ideal MacCready cruise/climb cycle."),
              nullptr,
              UpdateInfoBoxFinalETA,
              nullptr)

// e_WP_TimeLocal
INFOBOX_ENTRY(e_WP_TimeLocal,
              N_("Next arrival time"),
              N_("WP ETA"),
              N_("Estimated arrival local time at next waypoint, assuming "
                 "performance of ideal MacCready cruise/climb cycle."),
              IBFHelper<InfoBoxContentNextETA>::Create,
              nullptr,
              nullptr)

// e_WP_BearingDiff
INFOBOX_ENTRY(e_WP_BearingDiff,
              N_("Bearing difference"),
              N_("Brng D"),
              N_("Difference between the glider's track (direction of motion "
                 "across the ground) and the bearing to the next waypoint, or "
                 "for AAT tasks, the bearing to the target within the AAT "
                 "sector. GPS navigation is based on the track, and the track "
                 "may differ from the glider's heading when there is wind. "
                 "Chevrons point to the direction the glider needs to alter "
                 "course to correct the bearing difference, that is, to make "
                 "it so that the glider is tracking directly toward the next "
                 "waypoint. This calculation accounts for the curvature of "
                 "the Earth."),
              IBFHelper<InfoBoxContentBearingDiff>::Create,
              nullptr,
              nullptr)

// e_Temperature
INFOBOX_ENTRY(e_Temperature,
              N_("Outside air temperature"),
              N_("OAT"),
              N_("Outside air temperature measured by a probe if supported "
                 "by a connected intelligent variometer."),
              nullptr,
              UpdateInfoBoxTemperature,
              nullptr)

// e_HumidityRel
INFOBOX_ENTRY(e_HumidityRel,
              N_("Relative humidity"),
              N_("Rel Hum"),
              N_("Relative humidity of the air in percent as measured by a "
                 "probe if supported by a connected intelligent variometer."),
              nullptr,
              UpdateInfoBoxHumidity,
              nullptr)

// e_Home_Temperature
INFOBOX_ENTRY(e_Home_Temperature,
              N_("Forecast temperature"),
              N_("Max Temp"),
              N_("Forecast temperature of the ground at the home airfield, "
                 "used in estimating convection height and cloud base in "
                 "conjunction with outside air temperature and relative "
                 "humidity probe. (Touch-screen/PC only) Pressing the "
                 "up/down cursor keys adjusts this forecast temperature."),
              IBFHelper<InfoBoxContentTemperatureForecast>::Create,
              nullptr,
              nullptr)

// e_Fin_AA_Distance
INFOBOX_ENTRY(e_Fin_AA_Distance,
              N_("AAT distance around target"),
              N_("AAT Dtgt"),
              N_("Assigned Area Task distance around target points for "
                 "remainder of task."),
              nullptr,
              UpdateInfoBoxTaskAADistance,
              nullptr)

// e_AA_SpeedAvg
INFOBOX_ENTRY(e_AA_SpeedAvg,
              N_("AAT speed around target"),
              N_("AAT Vtgt"),
              N_("Assigned Area Task average speed achievable around target "
                 "points remaining in minimum AAT time."),
              nullptr,
              UpdateInfoBoxTaskAASpeed,
              nullptr)

// e_LD
INFOBOX_ENTRY(e_LD,
              N_("L/D vario"),
              N_("L/D Vario"),
              N_("Instantaneous lift/drag ratio, given by the indicated "
                 "airspeed divided by the total energy vertical speed, when "
                 "connected to an intelligent variometer. Negative values "
                 "indicate climbing cruise. If the total energy vario speed "
                 "is close to zero, the displayed value is '---'."),
              nullptr,
              UpdateInfoBoxLDVario,
              nullptr)

// e_Speed
INFOBOX_ENTRY(e_Speed,
              N_("Airspeed TAS"),
              N_("V TAS"),
              N_("True Airspeed reported by a supported external intelligent "
                 "vario."),
              nullptr,
              UpdateInfoBoxSpeed,
              nullptr)

// e_Team_Code
INFOBOX_ENTRY(e_Team_Code,
              N_("Team code"),
              N_("Team Code"),
              N_("Current Team code for this aircraft. Use this to report to "
                 "other team members. The last team aircraft code entered is "
                 "displayed underneath."),
              IBFHelper<InfoBoxContentTeamCode>::Create,
              nullptr,
              nullptr)

// e_Team_Bearing
INFOBOX_ENTRY(e_Team_Bearing,
              N_("Team bearing"),
              N_("Team Brng"),
              N_("Bearing to the team aircraft location at the last team "
                 "code report."),
              nullptr,
              UpdateInfoBoxTeamBearing,
              nullptr)

// e_Team_BearingDiff
INFOBOX_ENTRY(e_Team_BearingDiff,
              N_("Team bearing difference"),
              N_("Team BrngD"),
              N_("Relative bearing to the team aircraft location at the last "
                 "reported team code."),
              nullptr,
              UpdateInfoBoxTeamBearingDiff,
              nullptr)

// e_Team_Range
INFOBOX_ENTRY(e_Team_Range,
              N_("Team range"),
              N_("Team Dist"),
              N_("Range to the team aircraft location at the last reported "
                 "team code."),
              nullptr,
              UpdateInfoBoxTeamDistance,
              nullptr)

// e_CC_SpeedInst
INFOBOX_ENTRY(e_CC_SpeedInst,
              N_("Speed task instantaneous"),
              N_("V Task Inst"),
              N_("Instantaneous cross-country speed while on current task, "
                 "compensated for altitude. Equivalent to instantaneous "
                 "Pirker cross-country speed."),
              nullptr,
              UpdateInfoBoxTaskSpeedInstant,
              nullptr)

// e_Home_Distance
INFOBOX_ENTRY(e_Home_Distance,
              N_("Distance home"),
              N_("Home Dist"),
              N_("Distance to home waypoint (if defined)."),
              nullptr,
              UpdateInfoBoxHomeDistance,
              nullptr)

// e_CC_Speed
INFOBOX_ENTRY(e_CC_Speed,
              N_("Speed task achieved"),
              N_("V Task Ach"),
              N_("Achieved cross-country speed while on current task, "
                 "compensated for altitude. Equivalent to Pirker "
                 "cross-country speed remaining."),
              nullptr,
              UpdateInfoBoxTaskSpeedAchieved,
              nullptr)

// e_AA_TimeDiff
INFOBOX_ENTRY(e_AA_TimeDiff,
              N_("AAT delta time"),
              N_("AAT dT"),
              N_("Difference between estimated task time and AAT minimum "
                 "time. Coloured red if negative (expected arrival too "
                 "early), or blue if in sector and can turn now with "
                 "estimated arrival time greater than AAT time plus 5 "
                 "minutes."),
              nullptr,
              UpdateInfoBoxTaskAATimeDelta,
              nullptr)

// e_Climb_Avg
INFOBOX_ENTRY(e_Climb_Avg,
              N_("Thermal average over all"),
              N_("T Avg"),
              N_("Time-average climb rate in all thermals."),
              nullptr,
              UpdateInfoBoxThermalAllAvg,
              nullptr)

// e_RH_Trend
INFOBOX_ENTRY(e_RH_Trend,
              N_("Task req. total height trend"),
              N_("RH Trend"),
              N_("Trend (or neg. of the variation) of the total required "
                 "height to complete the task."),
              nullptr,
              UpdateInfoBoxVarioDistance,
              nullptr)

// e_Battery
INFOBOX_ENTRY(e_Battery,
              N_("Battery percent"),
              N_("Battery"),
              N_("Percentage of device battery remaining (where applicable) "
                 "and status/voltage of external power supply."),
              nullptr,
              UpdateInfoBoxBattery,
              nullptr)

// e_Fin_GR
INFOBOX_ENTRY(e_Fin_GR,
              N_("Final GR"),
              N_("Fin GR"),
              N_("Required glide ratio over ground to finish the task, given "
                 "by the distance to go divided by the height required to "
                 "arrive at the safety arrival height."),
              nullptr,
              UpdateInfoBoxFinalGR,
              nullptr)

// e_Alternate_1_Name
INFOBOX_ENTRY(e_Alternate_1_Name,
              N_("Alternate 1"),
              N_("Altn 1"),
              N_("Name and bearing to the best alternate landing location."),
              IBFHelperInt<InfoBoxContentAlternateName, 0>::Create,
              nullptr,
              nullptr)

// e_Alternate_2_Name
INFOBOX_ENTRY(e_Alternate_2_Name,
              N_("Alternate 2"),
              N_("Altn 2"),
              N_("Name and bearing to the second-best alternate landing "
                 "location."),
              IBFHelperInt<InfoBoxContentAlternateName, 1>::Create,
              nullptr,
              nullptr)

// e_Alternate_1_GR
INFOBOX_ENTRY(e_Alternate_1_GR,
              N_("Alternate 1 GR"),
              N_("Altn 1 GR"),
              N_("Geometric gradient to the arrival height above the best "
                 "alternate landing location. This is not adjusted for total "
                 "energy."),
              IBFHelperInt<InfoBoxContentAlternateGR, 0>::Create,
              nullptr,
              nullptr)

// e_H_QFE
INFOBOX_ENTRY(e_H_QFE,
              N_("Height above take-off"),
              N_("H T/O"),
              N_("Height based on an automatic take-off reference elevation "
                 "(like a QFE reference)."),
              nullptr,
              UpdateInfoBoxAltitudeQFE,
              altitude_infobox_panels)

// e_GR_Avg
INFOBOX_ENTRY(e_GR_Avg,
              N_("GR average"),
              N_("GR Avg"),
              N_("Distance flown during the configured averaging period "
                 "divided by the altitude lost during that period. Negative "
                 "values are shown as ^^^ and indicate climbing cruise ("
                 "height gain). For GR >200, the value is shown as +++. You "
                 "can configure the averaging period in the system setup "
                 "(suggested: 60, 90 or 120s). Lower values will be closer to "
                 "GR Inst, and higher values will be closer to GR Cruise. "
                 "Note: The distance is not the straight line between your "
                 "previous and current positions; it is the actual path "
                 "distance flown (including zigzags). This value is not "
                 "calculated while circling."),
              nullptr,
              UpdateInfoBoxGRAvg,
              nullptr)

// e_Experimental1
INFOBOX_ENTRY(e_Experimental1,
              N_("Experimental 1"),
              N_("Exp1"),
              NULL,
              nullptr,
              UpdateInfoBoxExperimental1,
              nullptr)

// e_OC_Distance
INFOBOX_ENTRY(e_OC_Distance,
              N_("Contest distance"),
              N_("Cont Dist"),
              N_("Instantaneous evaluation of the flown distance according "
                 "to the configured Contest rule set."),
              IBFHelper<InfoBoxContentContest>::Create,
              nullptr,
              nullptr)

// e_Experimental2
INFOBOX_ENTRY(e_Experimental2,
              N_("Experimental 2"),
              N_("Exp2"),
              NULL,
              nullptr,
              UpdateInfoBoxExperimental2,
              nullptr)

// e_CPU_Load
INFOBOX_ENTRY(e_CPU_Load,
              N_("CPU load"),
              N_("CPU"),
              N_("CPU load consumed by XCSoar averaged over 5 seconds."),
              nullptr,
              UpdateInfoBoxCPULoad,
              nullptr)

// e_WP_H
INFOBOX_ENTRY(e_WP_H,
              N_("Next altitude arrival"),
              N_("WP AltA"),
              N_("Absolute arrival altitude at the next waypoint in final "
                 "glide. For AAT tasks, the target within the AAT sector is "
                 "used."),
              IBFHelper<InfoBoxContentNextAltitudeArrival>::Create,
              nullptr,
              nullptr)

// e_Free_RAM
INFOBOX_ENTRY(e_Free_RAM,
              N_("Free RAM"),
              N_("Free RAM"),
              N_("Free RAM as reported by OS."),
              nullptr,
              UpdateInfoBoxFreeRAM,
              nullptr)

// e_FlightLevel
INFOBOX_ENTRY(e_FlightLevel,
              N_("Flight level"),
              N_("FL"),
              N_("Pressure Altitude given as Flight Level. If barometric "
                 "altitude is not available, FL is calculated from GPS "
                 "altitude, given that the correct QNH is set. In case the FL "
                 "is calculated from the GPS altitude, the FL label is "
                 "coloured red."),
              nullptr,
              UpdateInfoBoxAltitudeFlightLevel,
              altitude_infobox_panels)

// e_Barogram
INFOBOX_ENTRY(e_Barogram,
              N_("Barogram"),
              N_("Barogram"),
              N_("Trace of altitude during flight."),
              IBFHelper<InfoBoxContentBarogram>::Create,
              nullptr,
              nullptr)

// e_Vario_spark
INFOBOX_ENTRY(e_Vario_spark,
              N_("Vario trace"),
              N_("Vario Trace"),
              N_("Trace of vertical speed, as reported by the GPS, or the "
                 "intelligent vario total energy vario value if connected to "
                 "one."),
              IBFHelper<InfoBoxContentVarioSpark>::Create,
              nullptr,
              nullptr)

// e_NettoVario_spark
INFOBOX_ENTRY(e_NettoVario_spark,
              N_("Netto vario trace"),
              N_("Netto Trace"),
              N_("Trace of vertical speed of air-mass, equal to vario value "
                 "less the glider's estimated sink rate."),
              IBFHelper<InfoBoxContentNettoVarioSpark>::Create,
              nullptr,
              nullptr)

// e_CirclingAverage_spark
INFOBOX_ENTRY(e_CirclingAverage_spark,
              N_("Thermal climb trace"),
              N_("TC Trace"),
              N_("Trace of average climb rate each turn in circling, based of "
                 "the reported GPS altitude, or vario if available."),
              IBFHelper<InfoBoxContentCirclingAverageSpark>::Create,
              nullptr,
              nullptr)

// e_ThermalBand
INFOBOX_ENTRY(e_ThermalBand,
              N_("Climb band"),
              N_("Climb Band"),
              N_("Graph of average circling climb rate (horizontal axis) as a "
                 "function of altitude (vertical axis)."),
              IBFHelper<InfoBoxContentThermalBand>::Create,
              nullptr,
              nullptr)

// e_TaskProgress
INFOBOX_ENTRY(e_TaskProgress,
              N_("Task progress"),
              N_("Progress"),
              N_("Clock-like display of distance remaining along task, "
                 "showing achieved task points."),
              IBFHelper<InfoBoxContentTaskProgress>::Create,
              nullptr,
              nullptr)

// e_TaskMaxHeightTime
INFOBOX_ENTRY(e_TaskMaxHeightTime,
              N_("Time under max. start height"),
              N_("Start Height"),
              N_("Contiguous period during which the aircraft has been below "
                 "the task start maximum height."),
              nullptr,
              UpdateInfoBoxTaskTimeUnderMaxHeight,
              nullptr)

// e_Fin_ETE_VMG
INFOBOX_ENTRY(e_Fin_ETE_VMG,
              N_("Task time to go (ground speed)"),
              N_("Fin ETE VMG"),
              N_("Estimated time required to complete task, assuming current "
                 "ground speed is maintained."),
              nullptr,
              UpdateInfoBoxFinalETEVMG,
              nullptr)

// e_WP_ETE_VMG
INFOBOX_ENTRY(e_WP_ETE_VMG,
              N_("Next time to go (ground speed)"),
              N_("WP ETE VMG"),
              N_("Estimated time required to reach next waypoint, assuming "
                 "current ground speed is maintained."),
              IBFHelper<InfoBoxContentNextETEVMG>::Create,
              nullptr,
              nullptr)

// e_Horizon
INFOBOX_ENTRY(e_Horizon,
              N_("Attitude indicator"),
              N_("Horizon"),
              N_("Attitude indicator (artificial horizon) display calculated "
                 "from flight path, supplemented with acceleration and "
                 "variometer data if available."),
              IBFHelper<InfoBoxContentHorizon>::Create,
              nullptr,
              nullptr)

// e_NearestAirspaceHorizontal
INFOBOX_ENTRY(e_NearestAirspaceHorizontal,
              N_("Nearest airspace horizontal"),
              N_("Near AS H"),
              N_("Horizontal distance to the nearest airspace."),
              IBFHelper<InfoBoxNearestAirspaceHorizontal>::Create,
              nullptr,
              nullptr)

// e_NearestAirspaceVertical
INFOBOX_ENTRY(e_NearestAirspaceVertical,
              N_("Nearest airspace vertical"),
              N_("Near AS V"),
              N_("Vertical distance to the nearest airspace. A positive value "
                 "means the airspace is above you; a negative value means the "
                 "airspace is below you."),
              IBFHelper<InfoBoxNearestAirspaceVertical>::Create,
              nullptr,
              nullptr)

// e_WP_MC0AltDiff
INFOBOX_ENTRY(e_WP_MC0AltDiff,
              N_("Next MC0 altitude difference"),
              N_("WP MC0 AltD"),
              N_("Arrival altitude at the next waypoint with MC 0 setting "
                 "relative to the safety arrival height. For AAT tasks, the "
                 "target within the AAT sector is used."),
              IBFHelper<InfoBoxContentNextMC0AltitudeDiff>::Create,
              nullptr,
              nullptr)

// e_HeadWind
INFOBOX_ENTRY(e_HeadWind,
              N_("Wind, head component"),
              N_("Head Wind"),
              N_("Current head wind component. Head wind is calculated from "
                 "TAS and GPS ground speed if airspeed is available from an "
                 "external device; otherwise, the estimated wind is used."),
              IBFHelper<InfoBoxContentHeadWind>::Create,
              nullptr,
              nullptr)

// TerrainCollision
INFOBOX_ENTRY(TerrainCollision,
              N_("Terrain collision"),
              N_("Terr Coll"),
              N_("Distance to the next terrain collision along the current "
                 "task leg. At this location, the altitude will be below the "
                 "configured terrain clearance altitude."),
              nullptr,
              UpdateInfoBoxTerrainCollision,
              nullptr)

// NavAltitude
INFOBOX_ENTRY(NavAltitude,
              N_("Altitude (Auto)"),
              N_("Alt Auto"),
              N_("Barometric altitude obtained from a device equipped with a "
                 "pressure sensor, or GPS altitude if barometric altitude is "
                 "not available."),
              nullptr,
              UpdateInfoBoxAltitudeNav,
              altitude_infobox_panels)

// NextLegEqThermal
INFOBOX_ENTRY(NextLegEqThermal,
              N_("Thermal next leg equivalent"),
              N_("T Next Leg"),
              N_("Thermal climb rate on the next leg that is equivalent to a "
                 "thermal climb rate equal to the MacCready setting on the "
                 "current leg."),
              nullptr,
              UpdateInfoBoxNextLegEqThermal,
              nullptr)

// HeadWindSimplified
INFOBOX_ENTRY(HeadWindSimplified,
              N_("Wind, head component (simplified)"),
              N_("Head Wind *"),
              N_("Current head wind component. The simplified head wind is "
                 "calculated by subtracting GPS ground speed from TAS if "
                 "airspeed is available from an external device."),
              IBFHelper<InfoBoxContentHeadWindSimplified>::Create,
              nullptr,
              nullptr)

// CruiseEfficiency
INFOBOX_ENTRY(CruiseEfficiency,
              N_("Task cruise efficiency"),
              N_("Cruise Eff"),
              N_("Efficiency of cruise. 100 indicates perfect MacCready "
                 "performance. This value estimates your cruise efficiency "
                 "according to the current flight history with the set MC "
                 "value. Calculation begins after task is started."),
              nullptr,
              UpdateInfoBoxCruiseEfficiency,
              nullptr)

// WIND_ARROW
INFOBOX_ENTRY(WIND_ARROW,
              N_("Wind arrow"),
              N_("Wind"),
              N_("Wind speed estimated by XCSoar. Manual adjustment is "
                 "possible with the connected InfoBox dialogue. Pressing the "
                 "up/down cursor keys to cycle through settings, adjust the "
                 "values with left/right cursor keys."),
              IBFHelper<InfoBoxContentWindArrow>::Create,
              nullptr,
              nullptr)

// THERMAL_ASSISTANT
INFOBOX_ENTRY(THERMAL_ASSISTANT,
              N_("Thermal assistant"),
              N_("Thermal"),
              N_("Circular thermal assistant that shows the lift distribution "
                 "over each part of the circle."),
              IBFHelper<InfoBoxContentThermalAssistant>::Create,
              nullptr,
              nullptr)

// START_OPEN_TIME
INFOBOX_ENTRY(START_OPEN_TIME,
              N_("Start open/close countdown"),
              N_("Start open"),
              N_("Time left until the start point opens or closes."),
              nullptr,
              UpdateInfoBoxStartOpen,
              nullptr)

// START_OPEN_ARRIVAL_TIME
INFOBOX_ENTRY(START_OPEN_ARRIVAL_TIME,
              N_("Start open/close countdown at reaching"),
              N_("Start reach"),
              N_("Time left until the start point opens or closes, minus the "
                 "calculated time to reach the start point."),
              nullptr,
              UpdateInfoBoxStartOpenArrival,
              nullptr)

// NEXT_RADIAL
INFOBOX_ENTRY(NEXT_RADIAL,
              N_("Next radial"),
              N_("Radial"),
              N_("True bearing from the next waypoint to your position."),
              IBFHelper<InfoBoxContentRadial>::Create,
              nullptr,
              nullptr)

// ATC_RADIAL
INFOBOX_ENTRY(ATC_RADIAL,
              N_("ATC radial"),
              N_("ATC radial"),
              N_("Bearing from the selected reference location to your "
                 "position. The distance is displayed in nautical miles for "
                 "communication with ATC. If declination is entered, "
                 "magnetic bearing is given to match VOR radials."),
              nullptr,
              UpdateInfoBoxATCRadial,
              atc_infobox_panels)

// TASK_SPEED_HOUR
INFOBOX_ENTRY(TASK_SPEED_HOUR,
              N_("Speed task last hour"),
              N_("V Task H"),
              N_("Average cross-country speed while on current task over the "
                 "last hour, not compensated for altitude."),
              nullptr,
              UpdateInfoBoxTaskSpeedHour,
              nullptr)

// WP_NOMINAL_DIST
INFOBOX_ENTRY(WP_NOMINAL_DIST,
              N_("Next distance (nominal)"),
              N_("WP Dist-N"),
              N_("Distance to the currently selected waypoint. For AAT "
                 "tasks, this is the distance to the origin of the AAT "
                 "sector."),
              IBFHelper<InfoBoxContentNextDistanceNominal>::Create,
              nullptr,
              nullptr)

// CIRCLE_DIAMETER
INFOBOX_ENTRY(CIRCLE_DIAMETER,
              N_("Circle diameter"),
              N_("Circle D"),
              N_("Circle diameter. Displays estimated circle diameter and "
                 "full circle flight time. Useful for evaluating best "
                 "thermalling mode with a glider at different wing loading."),
              nullptr,
              UpdateInfoBoxCircleDiameter,
              nullptr)

// TAKEOFF_DISTANCE
INFOBOX_ENTRY(TAKEOFF_DISTANCE,
              N_("Distance takeoff"),
              N_("Takeoff Dist"),
              N_("Distance to where take-off was detected."),
              nullptr,
              UpdateInfoBoxTakeoffDistance,
              nullptr)

// CONTEST_SPEED
INFOBOX_ENTRY(CONTEST_SPEED,
              N_("Contest speed"),
              N_("Cont Speed"),
              N_("Instantaneous evaluation of the flown speed according to "
                 "the configured contest rule set."),
              IBFHelper<InfoBoxContentContestSpeed>::Create,
              nullptr,
              nullptr)

// FIN_MC0_ALTD
INFOBOX_ENTRY(FIN_MC0_ALTD,
              N_("Final MC0 altitude difference"),
              N_("Fin MC0 AltD"),
              N_("Arrival altitude at the final waypoint with MC 0 setting "
                 "relative to the safety arrival height."),
              nullptr,
              UpdateInfoBoxFinalMC0AltitudeDiff,
              nullptr)

// NEXT_ARROW
INFOBOX_ENTRY(NEXT_ARROW,
              N_("Next arrow"),
              N_("Next arrow"),
              N_("Arrow pointing to the currently selected waypoint. The "
                 "name of the waypoint and the distance are also shown. The "
                 "position used is the optimized position of the active "
                 "waypoint in the current task, the center of a selected goto "
                 "waypoint or the target within the AAT sector for AAT "
                 "tasks."),
              IBFHelper<InfoBoxContentNextArrow>::Create,
              nullptr,
              nullptr)

// e_WP_ETA_VMG
INFOBOX_ENTRY(e_WP_ETA_VMG,
              N_("Next waypoint arrival time (ground speed)"),
              N_("WP ETA VMG"),
              N_("Estimated arrival time at next waypoint, assuming current "
                 "ground speed is maintained."),
              IBFHelper<InfoBoxContentNextETAVMG>::Create,
              nullptr,
              nullptr)

// e_NonCircling_Climb_Perc
INFOBOX_ENTRY(e_NonCircling_Climb_Perc,
              N_("Percentage non-circling climb"),
              N_("% Str Climb"),
              N_("Percentage of time spent climbing without circling. These "
                 "statistics are reset upon starting the task."),
              nullptr,
              UpdateInfoBoxNonCirclingClimbRatio,
              nullptr)

// e_Climb_Perc_Chart
INFOBOX_ENTRY(e_Climb_Perc_Chart,
              N_("Percentage climb chart"),
              N_("Climb %"),
              N_("Pie chart of time circling and climbing, circling and "
                 "descending, and climbing non-circling."),
              IBFHelper<InfoBoxContentClimbPercent>::Create,
              nullptr,
              nullptr)

// e_NbrSat
INFOBOX_ENTRY(e_NbrSat,
              N_("Number of used satellites"),
              N_("Satellites"),
              N_("Number of satellites currently used by the GPS module. If "
                 "this information is unavailable, the displayed value is "
                 "'---'."),
              nullptr,
              UpdateInfoBoxNbrSat,
              nullptr)

// e_ActiveRadio
INFOBOX_ENTRY(e_ActiveRadio,
              N_("Active Radio Frequency"),
              N_("Act Freq"),
              N_("Active radio frequency."),
              IBFHelper<InfoBoxContentActiveRadioFrequency>::Create,
              nullptr,
              nullptr)

// e_StandbyRadio
INFOBOX_ENTRY(e_StandbyRadio,
              N_("Standby Radio Frequency"),
              N_("Stby Freq"),
              N_("Standby radio frequency."),
              IBFHelper<InfoBoxContentStandbyRadioFrequency>::Create,
              nullptr,
              nullptr)

// e_Thermal_Time
INFOBOX_ENTRY(e_Thermal_Time,
              N_("Thermal time"),
              N_("TC Time"),
              N_("Time spent in the current thermal."),
              nullptr,
              UpdateInfoBoxThermalTime,
              nullptr)

// e_Alternate_2_GR
INFOBOX_ENTRY(e_Alternate_2_GR,
              N_("Alternate 2 GR"),
              N_("Altn 2 GR"),
              N_("Geometric gradient to the arrival height above the "
                 "second-best alternate landing location. This is not "
                 "adjusted for total energy."),
              IBFHelperInt<InfoBoxContentAlternateGR, 1>::Create,
              nullptr,
              nullptr)

// e_HeartRate
INFOBOX_ENTRY(e_HeartRate,
              N_("Heart Rate"),
              N_("Heart"),
              N_("Heart rate in beats per minute."),
              nullptr,
              UpdateInfoBoxHeartRate,
              nullptr)

// e_TransponderCode
INFOBOX_ENTRY(e_TransponderCode,
              N_("Transponder Code"),
              N_("XPDR Code"),
              N_("Currently set transponder code."),
              IBFHelper<InfoBoxContentTransponderCode>::Create,
              nullptr,
              nullptr)

// e_EngineCHT
INFOBOX_ENTRY(e_EngineCHT,
              N_("Engine CHT"),
              N_("CHT"),
              N_("Engine cylinder head temperature."),
              nullptr,
              UpdateInfoBoxContentCHT,
              nullptr)

// e_EngineEGT
INFOBOX_ENTRY(e_EngineEGT,
              N_("Engine EGT"),
              N_("EGT"),
              N_("Engine exhaust gas temperature."),
              nullptr,
              UpdateInfoBoxContentEGT,
              nullptr)

// e_EngineRPM
INFOBOX_ENTRY(e_EngineRPM,
              N_("Engine Revolutions Per Minute"),
              N_("RPM"),
              N_("Engine revolutions per minute."),
              nullptr,
              UpdateInfoBoxContentRPM,
              nullptr)

// e_AAT_dT_or_ETA
INFOBOX_ENTRY(e_AAT_dT_or_ETA,
              N_("AAT dT and task ETA"),
              N_("AATdeltaOrETA"),
              N_("For AAT tasks: AAT delta time and estimated time of "
                 "arrival; for racing tasks: estimated time of arrival."),
              nullptr,
              UpdateInfoTaskETAorAATdT,
              nullptr)

// e_SpeedTaskEst
INFOBOX_ENTRY(e_SpeedTaskEst,
              N_("Speed task estimated"),
              N_("V Task Est"),
              N_("Estimated average cross-country speed for current task as "
                 "of task completion, assuming performance of ideal "
                 "MacCready cruise/climb cycle."),
              nullptr,
              UpdateInfoBoxTaskSpeedEst,
              nullptr)

// e_Home_AltDiff
INFOBOX_ENTRY(e_Home_AltDiff,
              N_("Home altitude difference"),
              N_("Home AltD"),
              N_("Arrival altitude at the home waypoint relative to the "
                 "safety arrival height."),
              nullptr,
              UpdateInfoBoxHomeAltitudeDiff,
              nullptr)

// e_SpeedTaskLeg
INFOBOX_ENTRY(e_SpeedTaskLeg,
              N_("Speed task leg"),
              N_("V Task Leg"),
              N_("Average cross-country speed while on current task leg, not "
                 "compensated for altitude."),
              nullptr,
              UpdateInfoBoxTaskSpeedLeg,
              nullptr)

// e_Alternate_1_AltDiff
INFOBOX_ENTRY(e_Alternate_1_AltDiff,
              N_("Alternate 1 altitude difference"),
              N_("Altn 1 AltD"),
              N_("Arrival altitude at the best alternate landing location "
                 "relative to the safety arrival height."),
              IBFHelperInt<InfoBoxContentAlternateAltDiff, 0>::Create,
              nullptr,
              nullptr)

// e_Alternate_2_AltDiff
INFOBOX_ENTRY(e_Alternate_2_AltDiff,
              N_("Alternate 2 altitude difference"),
              N_("Altn 2 AltD"),
              N_("Arrival altitude at the second-best alternate landing "
                 "location relative to the safety arrival height."),
              IBFHelperInt<InfoBoxContentAlternateAltDiff, 1>::Create,
              nullptr,
              nullptr)

