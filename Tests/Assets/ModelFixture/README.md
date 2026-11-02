# 고정 모델 회귀 입력

`CubeV2.bin`은 2026-10-30 로컬 `Assets/Models/BrickWallCube.bin`의 1,778바이트 스냅샷이다. Unity Exporter의 Binary v2이며 이미지 파일은 필요 없다. 데모 모델을 재익스포트해도 이 파일은 교체하지 않는다.

SHA-256: `ed1112ee907da73ffc6234e6b59d7071dea6d9631238211d0407d265942d859b`

ModelChecks의 손상·TRS·재질 변환 검사는 이 파일의 메모리 복사본만 변조한다. 원본 fixture는 읽기 전용이며 파일을 통한 검사에는 시스템 임시 파일을 사용한다. 실제 Assets/Models는 정상 로딩·GPU 비교에만 사용한다.

주요 바이트 위치(0부터): 버전 19, 첫 인덱스 1348, shadingModel 1515, colorSpace 1519, BaseColor 1523, surface 1575, UV scale/offset 1595, BaseColor Wrap U/V 1651/1655. TRS 검사는 31에 84바이트 Identity 부모를 삽입한 후 자식 행렬 144·첫 법선 552를 수정한다.

형식 변경으로 fixture를 교체해야 할 때만 ModelChecks의 크기·offset·기대값과 이 해시를 함께 갱신하고 정상 입력·각 거부 경로를 재검증한다. 일반 데모 재익스포트는 이 갱신 사유가 아니다.
