# Dear ImGui 공급 기록

- 원본: https://github.com/ocornut/imgui
- 릴리스: [v1.93.0](https://github.com/ocornut/imgui/releases/tag/v1.93.0)
- 고정 커밋: `c1aaac65e896a45f81f47a4d68f9959e0f86c859`
- 라이선스: [MIT](LICENSE.txt)
- 취득일: 2026-10-22

코어 `.cpp` 다섯 개(Demo 포함), 대응·내부·stb 헤더와 `imconfig.h`, Win32/DX12 백엔드, LICENSE만 위 커밋에서 취득했다. 나머지 examples·docs·백엔드는 포함하지 않는다. 공급자 구조·내용은 수정하지 않았다. [SHA256SUMS.txt](SHA256SUMS.txt)는 취득한 16개 원본 파일의 SHA-256이다.

재취득은 `https://raw.githubusercontent.com/ocornut/imgui/<고정 커밋>/<목록의 경로>`에서 한다. 업데이트 시 원본·해시·이 기록·프로젝트 의존성 문서를 함께 맞추고 두 구성의 Sandbox·ImGuiSmoke를 검증한다. 빌드 중 다운로드는 수행하지 않는다.

연동 설정은 [Build/ImGui.props](../../Build/ImGui.props), 수명·사용 범위와 공급자 실패 전달 한계는 [의존성 문서](../../docs/dependencies.md#dear-imgui)를 따른다. 프로젝트는 공급자 원본의 포맷·네임스페이스·PCH 정책을 변경하지 않는다.
