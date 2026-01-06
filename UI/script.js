// === DOM elements ===
const road = document.getElementById('road');
const scooter = document.getElementById('scooter');
const wheel = document.getElementById('wheel');
const speedSlider = document.getElementById('speed');
const speedMotorDisplay = document.getElementById('speed-motor');
const voltageDisplay = document.getElementById('voltage');
const currentDisplay = document.getElementById('current');
const torqueDisplay = document.getElementById('torque');
const loadCountDisplay = document.getElementById('load-count');
const convertPercentDisplay = document.getElementById('convert-percent');
const convertCircle = document.getElementById('convert-circle');
const canLog = document.getElementById('can-log');


// === Animation state ===
let roadScroll = 0;
let roadDirection = 1;
let duty = 0;
let sentSpeed = 0;
let canData = 0;

const dt = 0.04; // 40ms

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


// === Tham số động cơ và hệ số mô phỏng ===
const motorParams = {
  U_batt: 60,     // V
  omega0: 924,    // rpm no-load @100%
  v0: 120,    // km/h no-load @100%
  Tstall: 120,    // Nm
  Imax: 32.5,   // A
  Kt: 3.7,    // Nm/A
  Ifric0: 0,      // A @0%
  Ifric100: 2,      // A @100%
  R0: 0.5,    // Ω @25°C
  Tamb: 25,     // °C
  beta: 0.2     // °C per W
};
const {
  U_batt, omega0, v0, Tstall,
  Imax, Kt, Ifric0, Ifric100,
  R0, Tamb, beta
} = motorParams;


// State & target
const state = {
  rpm: 0,
  speed: 0,
  ifric: 0,
  iload: 0,
  current: 0,
  vsup: 0,
  pmech: 0,
  pelec: 0,
  tcoil: Tamb,
  stall: 'No',
  torque: 0,
  loadCount: 0,
  direction: 1
};
let target = { ...state };

// --- Tính toán target mỗi khi slider thay đổi ---
function computeTarget(duty, load) {
  const D = duty / 100; 
  const Tload = load;

  const Tmax = Tstall * D; 
  const r = Tmax > 0 ? Math.max(0, 1 - Tload / Tmax) : 0; 

  const Ifric = Ifric0 + (Ifric100 - Ifric0) * D;
  const Il = Tload / Kt;
  let Itot = Ifric + Il;
  if (Itot > Imax) Itot = Imax;

  let Vsup = U_batt * D - Itot * R0;
  if (Vsup < 0) Vsup = 0;

  const RPM = omega0 * D * r;
  const speed = v0 * D * r;

  const omegaRad = RPM * 2 * Math.PI / 60;
  const Pmech = r > 0 ? Tload * omegaRad : 0;
  const Pelec = Vsup * Itot;

  let Tcoil = Tamb + beta * (Pelec - Pmech);
  if (Tcoil < Tamb) Tcoil = Tamb;

  const stall = r <= 0 ? 'Yes' : 'No';

  const Torque = omegaRad > 0
    ? Pmech / omegaRad                       // T = P / ω
    : 0;

  updateStateSmooth();

  Object.assign(target, {
    rpm: RPM,
    speed,
    ifric: Ifric,
    iload: Il,
    current: Itot,
    vsup: Vsup,
    pmech: Pmech,
    pelec: Pelec,
    tcoil: Tcoil,
    stall,
    torque: Torque
  });
}

// --- Exponential smoothing (chỉ nội suy vào state) ---
const smoothing = 0.1;
function updateStateSmooth() {
  ['rpm', 'speed', 'ifric', 'iload', 'current', 'vsup', 'pmech', 'pelec', 'tcoil', 'torque']
    .forEach(k => {
      state[k] += (target[k] - state[k]) * smoothing;
    });
  state.stall = target.stall;
}



// --- Cập nhật UI ---
function updateUI() {
  speedMotorDisplay.textContent = `Vận tốc (km/h): ${state.speed.toFixed(0)}`;
  voltageDisplay.textContent = `${state.vsup.toFixed(1)} V`;
  currentDisplay.textContent = `${state.current.toFixed(1)} A`;
  torqueDisplay.textContent = `${state.torque.toFixed(1)} Nm`;
  loadCountDisplay.textContent = `${state.loadCount} vật`;
  convertPercentDisplay.textContent = `${state.tcoil.toFixed(1)} °C`;

  const safeTemp = 60;
  let pct = state.tcoil <= safeTemp ? (state.tcoil / safeTemp) * 100 : 100;
  let color = state.tcoil <= safeTemp ? 'green' : 'red';

  convertCircle.style.strokeDashoffset = 100 - pct;
  convertCircle.style.stroke = color;

}

// --- Animation bánh xe & nền đường ---
function updateScooter() {
  const maxSpeedForAnimation = 5;
  // Map tốc độ thực tế (rpm → km/h) sang animation speed
  const currentSpeedKmh = state.speed.toFixed(0);
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

  const sp = duty;
  const ld = state.loadCount * 11.5 + 5;

  console.log(`test: ${sp}`);

  computeTarget(sp, ld);
  updateUI();
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
const ctx = canvas.getContext("2d");
const slider = document.getElementById("duty");
const label = document.getElementById("duty-value");
let offset = 0;




function drawPWM() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  const period = 100;
  const high = duty;
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
  data[1] = speedValue & 0xFF; // Byte speed thấp
  data[2] = (speedValue >> 8) & 0xFF; // Byte speed cao
  // Các byte 3-7 giữ 0 mặc định

  if(data[1] === 0 && data[2] === 0) {
    data[0] = 0x04; // Command ID: Stop
  }

  const idStr = canId.toString(16).padStart(3, '0').toUpperCase();
  const dataStr = [...data].map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');

  return { id: idStr, dlc, data: dataStr, speed: sentSpeed, direction: data[3] };
}
let lastFrameStr = "";

function generateCANFrame() {
  // Truyền requestedSpeed làm tham số
  const { id, dlc, data, speed } = createCanFrameSetSpeed(sentSpeed);
  const idStr = `0x${id}`;
  let entry = '';

  if(speed > 0) {
    entry = `[EVCU] ID:${idStr} | DLC:${dlc} | Data:${data}  (${speed.toFixed(2)} Km/h)`;
  }
  else {
    entry = `[EVCU] ID:${idStr} | DLC:${dlc} | Data:${data} {Stop}` ;
  }

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


  if(dataBytes[0] !== '03' && dataBytes[0] !== '04') {
    alert('Chức năng hiện tại chỉ hỗ trợ mã lệnh Set Speed (03) và Stop (04).');
    return false;
  }

  let entry = '';

  if(dataBytes[0] === '03') {
    const speedLow = parseInt(dataBytes[1], 16);
    const speedHigh = parseInt(dataBytes[2], 16);
    const speedValue = speedLow + (speedHigh << 8);
    sentSpeed = speedValue / 100;
    entry = `[EVCU] ID:${id} | DLC:${dlc} | Data:${data} (${sentSpeed.toFixed(2)} Km/h)`;
  }

  else {
    entry = `[EVCU] ID:${id} | DLC:${dlc} | Data:${data} {Stop}` ;
  }  

  canData = `${id} ${dlc} ${data}`;

  const d = document.createElement("div");
  d.textContent = entry;
  canLog.appendChild(d);
  canLog.scrollTop = canLog.scrollHeight;
  input.value = '';
  return false;
}


// trước khi build payload, xác định Vref và ADC max
const VREF = 5, ADC_MAX = 4095;

// hàm chuyển giá trị x trong khoảng [0…X_MAX] sang ADC count
const toAdc = (x, X_MAX) => Math.round((x / X_MAX) * ADC_MAX);

// === Gộp fetch và send thành 1 hàm đồng bộ ===
async function syncData() {
  // 1) Lấy dữ liệu /data và cập nhật mô phỏng
  try {
    const res = await fetch('/data');   /* lấy dữ liệu từ data.csv */
    if (!res.ok) throw new Error(res.status);
    const d = await res.json();
    // Cập nhật requestedSpeed (có thể cập nhật thêm state khác nếu cần)
    duty = d.duty || duty;
     // 3) Chỉ đổi hướng khi direction thay đổi
    if (d.direction != roadDirection) {
      if (d.direction == 1) {
        turnRight();
      } else if (d.direction == -1) {
        turnLeft();
      }
      roadDirection = d.direction;
    }
    simulateAndUpdateUI();
  } catch (err) {
    console.error('fetch /data error:', err);
  }

  // 2) Chuẩn bị dữ liệu để gửi về /save-data
  // 2) Chuẩn bị data để gửi về /save-data
  const payload = {
    temp: toAdc(state.tcoil, 200),
    voltage: toAdc(state.vsup, 60),
    current: toAdc(state.current, 50),
    torque: toAdc(state.torque, 120),
    rpm: state.rpm.toFixed(0),
    can: canData,
  };

  // 3) Gửi dữ liệu mô phỏng về server
  try {
    //gửi payload xuống data.csv
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
