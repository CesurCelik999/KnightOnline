# NoticeUp V2 Release Candidate 1 — 2026-10-21

## Source identity
- Repository: CesurCelik999/KnightOnline
- Build branch: `build/gm-v3-noticeup-v2-20261021-1200`
- Last checked server-code commit: `e6680113c5d27a4c311a3d75f145930bac3c18e8`
- Builds: GitHub Actions > Build OpenKO Server Release, `windows-2022`, `msbuild Server.slnx /p:Configuration=Release /p:Platform=x64`
- The source includes previously implemented GM commands, including `+monsummon` and `+noticeall`.
- Pre-change GitHub source snapshot: `backup/server-pre-noticeup-v2-20261021-1200`
- Post-change GitHub source snapshot: `backup/server-post-noticeup-v2-20261021-1200`

## Files used
- `src/Server/Ebenezer/User.cpp`: NoticeUp V2 text parsing, header and packet
- `src/Server/Ebenezer/User.h`: `SendNoticeUp()` declaration
- `src/shared/packets.h`: `WIZ_NOTICE_UP = 0x7C`
- `NoticeUp-Config/notice_up.txt`: example configuration to copy next to `Ebenezer.exe` as `notice_up.txt`
- `src/Client/WarFare/UINoticeUp.*`: notice UI source, **not included in server-only build**
- `src/Client/WarFare/GameProcMain.cpp`: packet handler, **not included in server-only build**

## Expected preview behavior
Server title is taken from `@SERVER_NAME` plus the running Ebenezer `ZONE_INFO.MY_INFO` number, e.g. `MYKO | SERVER 1`.
A welcome line and manual `TITLE|TEXT`, `TITLE-TEXT`, or plain text lines follow.
`@EXP_EVENT=50` displays `EXP Event: %50`, `@NP_EVENT=OFF` hides NP, etc.
Maximum 8 total rows are transmitted.
**These event status values are manually configured, NOT linked to a live bonus scheduler.**
The current client-side NoticeUp source displays titles as clickable rows and shows descriptions on click; it does NOT always render every description expanded.
Current UI source places NoticeUp at `OffsetTop=160`, not flush with the screen top.

## Test / deployment checklist (Windows VPS only)
1. Open GitHub Actions for this exact build branch. Do not infer success from this source commit.
2. Confirm the build job is green and `OpenKO-Server-GM-V3-Release-x64` artifact exists before deployment.
3. On Windows VPS, record current `Ebenezer.exe` SHA-256 and make a verified timestamped executable/config backup. Stop Ebenezer cleanly.
4. Extract the artifact to a temporary staging folder. Verify `Ebenezer.exe` is present; compare version/hash and inspect the output.
5. Copy **only the required compatible binaries** and `notice_up.txt` (from `NoticeUp-Config`, not necessarily packaged in the artifact) to the runtime directory. Preserve existing INIs, DB files, and unrelated assets.
6. Restart services normally. Check logs for startup errors before logging in.
7. Test `MYKO | SERVER <ZONE_INFO.MY_INFO>`, welcome, manual rows, event ON/OFF values, and legacy `Notice.txt` separately.
8. For the game UI test, ensure the deployed **browser WASM/client** contains `WIZ_NOTICE_UP (0x7C)` parsing and `CUINoticeUp`. Rebuilding the server by itself cannot add that client code.
9. If incompatible, restore the previous backed-up `Ebenezer.exe` and `notice_up.txt`.

## Source synchronization and backup requirement
These GitHub snapshot branches DO NOT replace the required local canonical source backups in
`Documents/OpenKO-Source-Backups` on the Mac. Before further C++ source modifications,
take and verify date-hour-minute backups of both the pre-change and final source; reconcile
the GitHub branch with the Mac source and record a SHA-256 manifest.
