# Packet and Traffic Analysis Tool

## 프로젝트 개요
- **언어**: C++
- **기술 스택**: Npcap, ImGui
- **플랫폼**: Windows
- **주요 기능**: NIC(Network Interface Card)를 인식하고, 들어오는/나가는 트래픽을 캡처하여 분석하는 툴

---

## 담당 업무
- **UI 생성**: ImGui를 사용하여 사용자 인터페이스(UI) 설계 및 구현
- **Npcap 연동**: Npcap 라이브러리를 사용하여 네트워크 트래픽 캡처 기능 구현
- **전체 시스템 설계 및 구현**: 시스템 아키텍처 설계와 Npcap 및 ImGui 통합

---

## 시스템 기능

### 1. **NIC 인식**
- 현재 장치에 설치된 NIC(Network Interface Card)를 자동으로 탐지하여 표시합니다.

### 2. **트래픽 캡처**
- 선택된 NIC를 통해 들어오는 트래픽과 나가는 트래픽을 실시간으로 캡처합니다.

### 3. **트래픽 분석**
- 캡처된 네트워크 패킷을 분석하여 트래픽 패턴 및 다양한 네트워크 통계를 제공합니다.

### 4. **트래픽량 확인**
- 실시간으로 들어오는 트래픽과 나가는 트래픽의 양을 표시하여, 네트워크 사용량을 모니터링할 수 있습니다.

---

## 기술 스택 세부 사항

### 1. **Npcap**
- Npcap 라이브러리를 사용하여 네트워크 트래픽을 캡처하고, 로우레벨 패킷 데이터를 분석합니다.

### 2. **ImGui**
- **ImGui**를 활용하여 직관적인 UI를 구현하여, 사용자가 NIC 선택 및 트래픽 모니터링을 쉽게 할 수 있도록 구성합니다.
