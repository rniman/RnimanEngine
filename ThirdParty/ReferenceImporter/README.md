# ReferenceImporter 공급 기록

2026-10-30 ModelExporter Native/ReferenceImporter의 ExportData.h·ExportFormatReader.h·ExportFormatReader.cpp와 Documentation/FileFormat.md·FieldReference.md를 반영했다.
현재 v2만 지원하며 텍스처 바인딩의 Wrap U/V를 복원한다. Wrap이 없는 현재 v1도 재익스포트해야 한다. 과거 실험 포맷도 재익스포트가 필요하다. 공급 원본과 엔진 사본을 같은 v2 계약으로 갱신했다.

C++17 표준 라이브러리·예외 API·custom_asset_export namespace를 유지한다. 엔진은 C++20·PCH 없이 빌드한다. 별도 라이선스 파일은 확인하지 못했다.
엔진 사본은 공개 입력 대여·소유 반환 계약, 오류 검출 위치 설명, 생성자 중괄호 초기화, 내부 Reader 수명 설명과 메시·재질 ID 검증/삽입 분리를 보완한다.
[SHA256SUMS.txt](SHA256SUMS.txt)는 이 보완을 포함한 사본의 해시다.

Reader는 파싱·검증만 수행한다. textureRef는 확장자 없는 파일명이며 실제 폴더·확장자 선택은 Sandbox가 담당한다. EngineIntegration 예제는 포함하지 않는다.
[규격](FileFormat.md) · [개발 기록](../../docs/development.md)

2026-10-30 Wrap v2 갱신: 엔진 사본의 기존 초기화·오류 처리·참조 검증 보완을 유지하고 Version guard·Texture 최소 레코드 크기·wrapU/V 읽기와 범위 검증을 추가했다. 공급 원본도 같은 계약으로 갱신했다.

2026-10-30 재질 조회 통합: 원본 Reader의 materialIndices 단일 조회·UV/접선 검증 순회 통합·조회 예산 64바이트를 반영했다. 엔진 사본의 ID 검증/삽입 분리와 기존 API 주석·초기화는 유지한다. 파일 형식은 v2 그대로이며 이번 내부 정리로 인한 재익스포트는 필요 없다. 원본의 Submesh.index 제거는 이번 재질 조회 변경과 별개이므로 반영하지 않았다.
