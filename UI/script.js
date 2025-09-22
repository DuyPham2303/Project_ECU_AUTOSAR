  // === DOM elements ===
  const road           = document.getElementById('road');
  const scooter        = document.getElementById('scooter');
  const wheel          = document.getElementById('wheel');
  const speedSlider    = document.getElementById('speed');
  const speedMotorDisplay    = document.getElementById('speed-motor');
  const voltageDisplay = document.getElementById('voltage');
  const currentDisplay = document.getElementById('current');
  const torqueDisplay  = document.getElementById('torque');
  const loadCountDisplay = document.getElementById('load-count');
  const convertPercentDisplay = document.getElementById('convert-percent');
  const convertCircle   = document.getElementById('convert-circle');
  const canLog          = document.getElementById('can-log');

  // === Tham số động cơ và hệ số mô phỏng ===
  const motorParams = {
    nominalVoltage: 60,
    maxSpeed: 924,           // rpm ~ 120 km/h
    maxTorque: 120,
    Kt: 3.7,
    maxCurrent: 32.5,
    internalResistance: 0.5,
    thermalCoeff: 0.02,
    coolingRate: 0.1,
    ambientTemp: 25,
    maxTemp: 155,
  };

  // === Đối tượng trạng thái động cơ ===
  const state = {
    speed: 0,
    torque: 0,
    current: 0,
    voltage: motorParams.nominalVoltage,
    temperature: motorParams.ambientTemp,
    pwmDuty: 0,
    loadCount: 0,
    direction: 1,
  };

  // Trạng thái mục tiêu
  let targetState = {
    speed: 0,
    torque: 0,
    current: 0,
    voltage: motorParams.nominalVoltage,
    temperature: motorParams.ambientTemp,
    pwmDuty: 0,
    direction: 1,
  };

  // === PID Controller ===
  const Kp = 1.0;
  const Ki = 0.1;
  const Kd = 0.05;
  let integral = 0;
  let previousError = 0;
  const dt = 0.04; // 40ms

  function computePID(targetSpeed, actualSpeed) {
    const error = targetSpeed - actualSpeed;
    integral += error * dt;
    const derivative = (error - previousError) / dt;
    const output = Kp * error + Ki * integral + Kd * derivative;
    previousError = error;
    return Math.min(Math.max(output, 0), 100);
  }

  // === Animation state ===
  let roadScroll = 0;
  let roadDirection = 1;
  let requestedSpeed = 0;
  let sentSpeed = 0;
  let canData = 0;

  // --- Map slider [0.5..5] to speed km/h [0..120] ---
  function getSpeedKmhFromSlider() {
    const val = parseFloat(speedSlider.value);
    const minInput = 0.5;
    const maxInput = 5;
    const minOutput = 0;
    const maxOutput = 120;
    const mapped = ((val - minInput) / (maxInput - minInput)) * (maxOutput - minOutput) + minOutput;
    return Math.min(Math.max(mapped, minOutput), maxOutput);
  }

  // --- Cập nhật nhiệt độ động cơ ---
  function updateTemperature(deltaTime = dt) {
    const ambientTemp = 27;
    const currentWeight = 1.0;
    const torqueWeight = 0.5;
    const speedWeight = 0.3;

    const currentFactor = Math.min(state.current / motorParams.maxCurrent, 1);
    const torqueFactor = Math.min(state.torque / motorParams.maxTorque, 1);
    const speedFactor = Math.min(Math.abs(state.speed) / motorParams.maxSpeed, 1);

    const thermalRiseFactor = 
      currentWeight * currentFactor + 
      torqueWeight * torqueFactor + 
      speedWeight * speedFactor;

    const tempIncrease = thermalRiseFactor * motorParams.thermalCoeff * state.current * deltaTime;
    const tempDecrease = motorParams.coolingRate * (state.temperature - ambientTemp) * deltaTime;

    let newTemp = state.temperature + tempIncrease - tempDecrease;
    state.temperature = Math.min(Math.max(newTemp, ambientTemp), motorParams.maxTemp + 20);
  }

  // --- Tính trạng thái mục tiêu dựa vào tốc độ và tải ---
  function calculateTargetState(speedKmh, loadNm) {
    targetState.direction = speedKmh < 0 ? -1 : 1;
    const maxKmh = 120;
    const absKmh = Math.min(Math.max(Math.abs(speedKmh), 0), maxKmh);
    const targetRpm = (absKmh / maxKmh) * motorParams.maxSpeed;

    if (loadNm <= motorParams.maxTorque && absKmh > maxKmh) {
      // Max speed, có tải nhẹ → tốc độ thực tế giảm do tải
      const loadFactor = loadNm / motorParams.maxTorque;
      const speedReductionFactor = Math.max(0.6, 1 - loadFactor * 0.4);
      const actualRpm = targetRpm * speedReductionFactor;

      targetState.speed = actualRpm * targetState.direction;
      targetState.torque = loadNm;

      const calculatedCurrent = targetState.torque / motorParams.Kt;
      targetState.current = Math.min(calculatedCurrent, motorParams.maxCurrent * 1.1);

      targetState.voltage = motorParams.nominalVoltage - targetState.current * motorParams.internalResistance;

      const pidPwm = computePID(targetRpm, actualRpm);
      targetState.pwmDuty = Math.min(pidPwm, 100);
    } 
    else if (loadNm > motorParams.maxTorque) {
      // Quá tải, stall
      targetState.speed = 0;
      targetState.torque = loadNm;
      targetState.current = Math.min(targetState.torque / motorParams.Kt, motorParams.maxCurrent + 10);
      targetState.voltage = motorParams.nominalVoltage - targetState.current * motorParams.internalResistance;
      targetState.pwmDuty = 100;
    } 
    else {
      // Tải cao hoặc tăng tốc
      const availableRpm = motorParams.maxSpeed * (1 - loadNm / motorParams.maxTorque);
      const cappedRpm = Math.max(Math.min(targetRpm, availableRpm), 0);

      targetState.speed = cappedRpm * targetState.direction;
      targetState.torque = loadNm;
      targetState.current = Math.min(targetState.torque / motorParams.Kt, motorParams.maxCurrent);
      targetState.voltage = motorParams.nominalVoltage - targetState.current * motorParams.internalResistance;
      targetState.pwmDuty = (Math.abs(targetState.speed) / motorParams.maxSpeed) * 100;
    }
  }

  // --- Nội suy mượt giữa trạng thái hiện tại và mục tiêu ---
  function smoothUpdate(currentValue, targetValue, step = 0.05) {
    const delta = targetValue - currentValue;
    if (Math.abs(delta) < 0.001) return targetValue;
    return currentValue + delta * step;
  }

  // --- Cập nhật trạng thái thực tế ---
  function updateStateSmooth() {
    state.speed = smoothUpdate(state.speed, targetState.speed, 0.1);
    state.torque = smoothUpdate(state.torque, targetState.torque, 0.1);
    state.current = smoothUpdate(state.current, targetState.current, 0.1);
    state.voltage = smoothUpdate(state.voltage, targetState.voltage, 0.05);
    state.pwmDuty = smoothUpdate(state.pwmDuty, targetState.pwmDuty, 0.1);
    state.direction = targetState.direction;

    updateTemperature(dt);
  }

  // --- Cập nhật UI ---
  function updateUI() {
    speedMotorDisplay.textContent = `Vận tốc (km/h): ${ (Math.abs(state.speed) / motorParams.maxSpeed * 120).toFixed(1) }`;
    voltageDisplay.textContent = `${state.voltage.toFixed(1)} V`;
    currentDisplay.textContent = `${state.current.toFixed(1)} A`;
    torqueDisplay.textContent = `${state.torque.toFixed(1)} Nm`;
    loadCountDisplay.textContent = `${state.loadCount} vật`;
    convertPercentDisplay.textContent = `${state.temperature.toFixed(1)} °C`;

    const safeTemp = 60;
    let pct = state.temperature <= safeTemp ? (state.temperature / safeTemp) * 100 : 100;
    let color = state.temperature <= safeTemp ? 'green' : 'red';

    convertCircle.style.strokeDashoffset = 100 - pct;
    convertCircle.style.stroke = color;

  }

  // --- Animation bánh xe & nền đường ---
  function updateScooter() {
    const maxSpeedForAnimation = 5;
    // Map tốc độ thực tế (rpm → km/h) sang animation speed
    const currentSpeedKmh = (Math.abs(state.speed) / motorParams.maxSpeed) * 120;
    const actualSpeed = (currentSpeedKmh / 120) * maxSpeedForAnimation;

    if (actualSpeed === 0) {
      wheel.style.animationPlayState = 'paused';
    } else {
      wheel.style.animationPlayState = 'running';
      wheel.style.animationDuration = `${1 / actualSpeed}s`;
      roadScroll += state.direction * actualSpeed * 2;
      road.style.backgroundPositionX = `${roadScroll}px`;
    }
  }

  // --- Hàm chạy mô phỏng và cập nhật liên tục ---
  function simulateAndUpdateUI() {
    const sp = requestedSpeed;
    const ld = state.loadCount * 12 + 5;

    calculateTargetState(sp, ld);
    updateStateSmooth();
    updateUI();
    //updateScooter();
  }

  // --- Thao tác tải ---
  function addLoad() {
    if (state.loadCount >= 10) return;
    const newLoad = document.createElement('div');
    newLoad.classList.add('load');
    newLoad.style.left = `${30 + state.loadCount * 25}px`;
    scooter.appendChild(newLoad);
    state.loadCount++;
  }
  function removeLoad() {
    if (state.loadCount <= 0) return;
    const loads = scooter.querySelectorAll('.load');
    scooter.removeChild(loads[loads.length - 1]);
    state.loadCount--;

  }

  // --- Đổi chiều ---
  function turnLeft() {
    roadDirection = -1;
    wheel.classList.add('reverse');
    scooter.style.transform = 'scaleX(-1)';
    state.direction = -1;

  }
  function turnRight() {
    roadDirection = 1;
    wheel.classList.remove('reverse');
    scooter.style.transform = 'scaleX(1)';
    state.direction = 1;

  }

  // --- Khởi động mô phỏng ---
  simulateAndUpdateUI();

  updateScooter();
  setInterval(simulateAndUpdateUI, 500);

  setInterval(updateScooter, dt * 1000);


// === PWM canvas giữ nguyên ===
const canvas = document.getElementById("canvas");
const ctx    = canvas.getContext("2d");
const slider = document.getElementById("duty");
const label  = document.getElementById("duty-value");
let offset   = 0;

function map120to100(value) {
    if (value <= 0) return 0;
    if (value >= 120) return 100;
    return (value / 120) * 100;
}
  

function drawPWM() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  const period = 100;
  const high = map120to100(requestedSpeed);
  ctx.beginPath();
  ctx.lineWidth = 2;
  ctx.strokeStyle = "#00ccff";
  let x = canvas.width + offset;
  const yH = canvas.height * 0.25;
  const yL = canvas.height * 0.75;
  ctx.moveTo(x, yL);
  while (x > -period) {
    ctx.lineTo(x, yL);
    ctx.lineTo(x, yH);
    ctx.lineTo(x - high, yH);
    ctx.lineTo(x - high, yL);
    ctx.lineTo(x - period, yL);
    x -= period;
  }
  ctx.stroke();
}
function animatePWM() {
  drawPWM();
  offset = (offset + 2) % 100;
  requestAnimationFrame(animatePWM);
}

animatePWM();

// === CAN log giả lập ===

// Hàm tạo frame Set Speed với tham số speed (km/h)
function createCanFrameSetSpeed(sentSpeed) {
  const canId = 0x3F0;
  const dlc = 8;
  const data = new Uint8Array(dlc);

  const speedValue = Math.round(sentSpeed * 100);

  data[0] = 0x03;              // Command ID: Set Speed
  data[1] = speedValue & 0xFF; // Byte thấp
  data[2] = (speedValue >> 8) & 0xFF; // Byte cao
  // Các byte 3-7 giữ 0 mặc định

  const idStr = canId.toString(16).padStart(3, '0').toUpperCase();
  const dataStr = [...data].map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');

  return { id: idStr, dlc, data: dataStr, speed: sentSpeed };
}
let lastFrameStr = "";

function generateCANFrame() {
  // Truyền requestedSpeed làm tham số
  const { id, dlc, data, speed } = createCanFrameSetSpeed(sentSpeed);
  const idStr = `0x${id}`;
  const entry = `[EVCU] ID:${idStr} | DLC:${dlc} | Data:${data}  (${speed.toFixed(2)} Km/h)`;

  if (entry !== lastFrameStr) {
    lastFrameStr = entry;
    canData = `${idStr} ${dlc} ${data}`;
    const d = document.createElement('div');
    d.textContent = entry;
    canLog.appendChild(d);
    canLog.scrollTop = canLog.scrollHeight;
  }
}


let debounceTimeout = null;

speedSlider.addEventListener('input', () => {
  // Xóa timeout trước nếu còn
  if (debounceTimeout) clearTimeout(debounceTimeout);

  // Đặt timeout 1 giây mới cập nhật requestedSpeed và gọi generateCANFrame
  debounceTimeout = setTimeout(() => {
    sentSpeed = getSpeedKmhFromSlider();
    generateCANFrame();
    debounceTimeout = null;
  }, 1000);
});




function sendCanCode() {
  const input = document.getElementById('can-code');
  const code = input.value.trim();

  if (!code) {
    alert('Vui lòng nhập mã CAN hợp lệ.');
    return false;
  }

  const parts = code.split(/\s+/);
  if (parts.length < 3) {
    alert('Mã CAN phải gồm ID, DLC và dữ liệu.');
    return false;
  }

  const id = parts[0];
  if (!/^0x[0-9A-Fa-f]{3}$/.test(id)) {
    alert('ID phải có dạng 0xNNN (3 ký tự hex). Ví dụ: 0x3F0');
    return false;
  }

  const dlc = parts[1];
  if (!/^(0?[1-8])$/.test(dlc)) {
    alert('DLC phải là số từ 1 đến 8, có thể có số 0 đứng trước, ví dụ: 8 hoặc 08.');
    return false;
  }
  const dlcVal = parseInt(dlc, 10);

  const dataBytes = parts.slice(2);
  if (dataBytes.length !== dlcVal) {
    alert(`Số byte dữ liệu phải bằng DLC (${dlcVal}).`);
    return false;
  }
  for (const b of dataBytes) {
    if (!/^[0-9A-Fa-f]{2}$/.test(b)) {
      alert(`Byte dữ liệu không hợp lệ: "${b}". Mỗi byte phải 2 ký tự hex.`);
      return false;
    }
  }

  // Format lại data string chuẩn
  const data = dataBytes.map(b => b.toUpperCase()).join(' ');


  const entry = `[EVCU] ID:${id} | DLC:${dlcVal} | Data:${data} {Stop}`;

  canData = `${id} ${dlc} ${data}`;


  const d = document.createElement("div");
  d.textContent = entry;
  canLog.appendChild(d);
  canLog.scrollTop = canLog.scrollHeight;
  input.value = '';
  return false;
}

// === Gộp fetch và send thành 1 hàm đồng bộ ===
async function syncData() {
  // 1) Lấy dữ liệu /data và cập nhật mô phỏng
  try {
    const res = await fetch('/data');
    if (!res.ok) throw new Error(res.status);
    const d = await res.json();
    // Cập nhật requestedSpeed (có thể cập nhật thêm state khác nếu cần)
    requestedSpeed = d.requestedSpeed || requestedSpeed;
    simulateAndUpdateUI();
  } catch (err) {
    console.error('fetch /data error:', err);
  }

  // 2) Chuẩn bị dữ liệu để gửi về /save-data
  const payload = {
    temp: state.temperature.toFixed(2),
    voltage: state.voltage.toFixed(2),
    current: state.current.toFixed(2),
    torque: state.torque.toFixed(2),
    currentSpeed: (Math.abs(state.speed) / motorParams.maxSpeed * 120).toFixed(2),
    can: canData,
  };

  // 3) Gửi dữ liệu mô phỏng về server
  try {
    const res2 = await fetch('/save-data', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });
    if (!res2.ok) {
      console.error('Lỗi response /save-data:', res2.status);
    } else {
      const text = await res2.text();
      console.log('Server trả về:', text);
    }
  } catch (err) {
    console.error('Lỗi gửi /save-data:', err);
  }
}

// Gọi ngay lần đầu, sau đó mỗi 1s
syncData();
setInterval(syncData, 1000);
