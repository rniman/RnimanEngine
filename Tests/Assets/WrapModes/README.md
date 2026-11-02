# Wrap U/V 회귀 입력

`surface_wrap_U_V.bin` 9개는 외부 Unity Exporter가 생성한 Binary v2다. 2026-10-30에 `D:/DevelopTool/ModelExporter/Exporter/Logs/ExporterVerification/UnityMaterials`의 동명 파일과 바이트 일치를 확인했다. 파일명의 U/V는 0 Repeat, 1 ClampToEdge, 2 MirroredRepeat이며 축별 3×3 조합을 포함한다.

RendererSmoke의 `CheckTextureAddressing`은 실제 로더로 BIN을 읽고 2×2 메모리 이미지를 연결한다. 범위 밖 UV의 네 지점에서 예상 texel을 GPU readback으로 비교하고, 모델 선택을 바꿨다가 되돌려 Sampler 선택도 확인한다. 외부 텍스처 파일은 필요 없다.

재생성 시 외부 Exporter에서 같은 조합과 UV를 유지한다. BIN의 버전 번호나 Wrap 바이트만 바꿔 기준 입력을 만들지 않는다. 잘못된 Wrap 값과 이전 v1의 거부는 `ModelChecks.cpp`에서 별도 변조 입력으로 검사한다.
