# CSG electrical assessment submission

Open reports/CSGR27_Electrical_Task_Hanjing Tang.pdf for the combined submission.
The editable Word version is beside it. Main answers are followed by external
references and appendices containing the prepared diagrams, simulation evidence,
code, component list and reproduction notes.

## D implementation
src/logic.c and SUMMARY.md are copied unchanged from the current working project.
The rest of the C harness is supplied by CSG Racing. README.md and TASK.md are
the original team instructions. The native output in evidence/D/native_results.txt
was generated from this packaged source. Wokwi web simulation has been run. The web adapter and simulation copy are in wokwi_web/, and the two supplied screenshots are in evidence/D/. See the report for results and limitations.

From a terminal with CMake and a C compiler available:
    cmake -S . -B build-native -DNATIVE_BUILD=ON
    cmake --build build-native --config Debug
Run build-native/embedded_task or, for a Visual Studio generator,
build-native/Debug/embedded_task.exe.

## B simulation
Load one file from simulations/B_precharge/ in Multisim XSPICE.
    source <actual-full-path-to-the-cir-file>
    tran 100u 1.2 0 100u uic
    plot v(1) v(2)
    plot v(3)
Node 1 is pack voltage; node 2 is bus voltage; node 3 is current monitor
(1 V = 1 A). Replace the placeholder with an actual path.

## C simulation
The V5 decks are in simulations/C_driver/. An example normal run is:
    source <actual-full-path-to-C_V5_Normal.cir>
    tran 10u 100m 0 1u
    plot v(7) v(6)
    plot i(lcoil)
    plot v(5)
    plot v(25) v(26)
For coil differential voltage use C_V5_Normal_Monitors.cir and plot v(27).
The report explains the model limits. No physical or HV testing is claimed.

## Procurement and evidence
components/C_BOM.csv has part identifiers and distributor links without live
stock claims. C_BOM_original_snapshot.csv retains the earlier dated stock record.
Academic/manufacturer sources are in the report's external-reference list.
Prepared diagrams, netlists, screenshots and code are project evidence instead.
The published Badawy Figure 11 is marked as an external CC BY 4.0 figure.

## Before submission

The final report names Hanjing Tang and ranks LV hardware, Embedded and HV in that order.

## Uploading
This folder contains no .git history, .vs cache, build outputs or executables.
For the existing individual GitHub repository, copy these files into that
checkout, review the changes, commit and push; do not create a replacement
history just to upload the folder. Ensure src/logic.c and SUMMARY.md are visible
in the public repository. The ZIP can be sent as accompanying files to the team.
The D task still requires the public repository URL as its submission.

Existing individual repository: https://github.com/hugo060311-web/HT-csgr27-electrical-task
The packaged files have not been uploaded or pushed by this preparation step.
