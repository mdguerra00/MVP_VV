# Project Notes

## User Variable Mapping

The UnicView project uses the following user-variable addresses. The mapping was verified against `mvp.uvs` (UnicView/V_Vision project file) and the constants defined in `MVP/user_variables.h`.

| Address | Designator        | Type  | Description |
|---------|-------------------|-------|-------------|
| 121 | Main_Screen      | S32   | current screen ID |
| 122 | txt_Config       | String| "Settings" label |
| 123 | Lang             | S32   | language selector 0=EN,1=PT,2=ES,3=DE |
| 124 | txt_Start_Cure   | String| "Start Cure" label |
| 125 | txt_Lang         | String| "Language" label |
| 126 | Lista_de_Idiomas | S32   | language list index |
| 127 | txt_Admin        | String| "Admin" label |
| 128 | txt_System       | String| "System Information" label |
| 129 | Start_Glaze_Cure | String| "Start Glaze Cure" label |
| 130 | Pre_Cure_1       | S32   | pre-cure step 1 duration (s) |
| 131 | Pre_Cure_2       | S32   | pre-cure step 2 duration (s) |
| 132 | Pre_Cure_3       | S32   | pre-cure step 3 duration (s) |
| 133 | Pre_Cure_4       | S32   | pre-cure step 4 duration (s) |
| 134 | Pre_Cure_5       | S32   | pre-cure step 5 duration (s) |
| 135 | Pre_Cure_6       | S32   | pre-cure step 6 duration (s) |
| 136 | Pre_Cure_7       | S32   | pre-cure step 7 duration (s) |
| 137 | txt_Seconds      | String| "Seconds" label |
| 138 | Selected_Pre_Cure| S32   | selected pre-cure duration (s) |
| 139 | Time_Curando     | S32   | elapsed cure time (s) |
| 140 | Timer_Start_Stop | S32   | timer state 0–stop,1–start,3–pause |
| 141 | progress_permille| S32   | progress bar value (0–1000) |


Addresses 129–137 are reserved for additional presets and labels; see `MVP/user_variables.h` for details.
