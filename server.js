// server.js (corrigido)
const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const path = require('path');

const PORT = process.env.PORT || 3000;
const SERIAL_PATH = process.env.SERIAL_PATH || 'COM3'; // ajuste por ENV
const SERIAL_BAUD = Number(process.env.SERIAL_BAUD || 115200  );

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// ✅ serve index.html, styles.css e afins do diretório do projeto
app.use(express.static(__dirname));
// scripts do cliente
app.use('/public', express.static(path.join(__dirname, 'public')));

// rota padrão
app.get('/', (_req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

// pequeno buffer de histórico
const ring = [];
const RING_MAX = 500;
function pushRing(item) {
  ring.push(item);
  if (ring.length > RING_MAX) ring.shift();
}

io.on('connection', (socket) => {
  socket.emit('bootstrap', ring);
});

// timestamp utilitário
function nowString() {
  const d = new Date();
  const pad = (n) => String(n).padStart(2, '0');
  return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())} ${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
}

// === Serial ===
let port;
try {
  port = new SerialPort({ path: SERIAL_PATH, baudRate: SERIAL_BAUD });
} catch (e) {
  console.error('Falha ao abrir porta serial:', e.message);
}

if (port) {
  const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));
  parser.on('data', (line) => {
    const raw = line.trim();
    // log básico para você ver no terminal
    console.log('SERIAL>', raw);
    try {
      const obj = JSON.parse(raw);
      const durationMin = obj.duration != null ? Number(obj.duration)
        : (obj.duration_ms != null ? (Number(obj.duration_ms) / 60000) : 1);
      const intensity = obj.intensity != null ? Number(obj.intensity)
        : (obj.peak_intensity != null ? Number(obj.peak_intensity) : 0);

      const t_ms = obj.t_ms != null ? Number(obj.t_ms)
        : 0;
      const dt_ms = obj.dt_ms != null ? Number(obj.dt_ms)
        : 0;
        
      const out = {
        timestamp: obj.timestamp || nowString(),
        events: Number(obj.events || 1),
        duration: Number.isFinite(durationMin) ? Number(durationMin) : 1,
        intensity: Number.isFinite(intensity) ? intensity : 0,
        t_ms: Number.isFinite(t_ms) ? t_ms : 0,
        dt_ms: Number.isFinite(dt_ms) ? dt_ms : 0,
      };
      pushRing(out);
      io.emit('reading', out);
      // e também loga o que foi para o browser
      console.log('EMIT >', out);
    } catch {
      // ignora linhas que não são JSON (ex.: "DBG ..." do Arduino)
    }
  });

  port.on('open', () => console.log(`Serial aberta em ${SERIAL_PATH} @ ${SERIAL_BAUD}`));
  port.on('error', (e) => console.error('Erro serial:', e.message));
} else {
  console.warn('⚠️ Sem porta serial aberta. Verifique SERIAL_PATH.');
}

server.listen(PORT, () => console.log(`Servidor em http://localhost:${PORT}`));
