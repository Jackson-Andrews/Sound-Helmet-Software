const http = require('http');
const { spawn } = require('child_process');

const PORT = 8000;

const server = http.createServer((req, res) => {
  // Set headers for Opus over Ogg streaming.
  res.writeHead(200, {
    'Content-Type': 'audio/ogg',
    'Transfer-Encoding': 'chunked',
    Connection: 'keep-alive',
  });

  // Spawn ffmpeg to capture audio and stream it as Opus.
  const ffmpeg = spawn('ffmpeg', [
    '-f',
    'avfoundation',
    '-probesize',
    '32k',
    '-analyzeduration',
    '0',
    '-i',
    ':BlackHole 64ch',
    '-filter_complex',
    'pan=stereo|c0=c0|c1=c1',
    '-ar',
    '48000',
    '-c:a',
    'libopus',
    '-application',
    'audio',
    '-frame_duration',
    '20',
    '-b:a',
    '64k',
    '-bufsize',
    '32k',
    '-f',
    'ogg',
    'pipe:1',
  ]);

  ffmpeg.stdout.on('data', (chunk) => {
    const canContinue = res.write(chunk);
    if (!canContinue) {
      ffmpeg.stdout.pause();
      res.once('drain', () => ffmpeg.stdout.resume());
    }
  });

  ffmpeg.stderr.on('data', (data) => {
    console.error('[ffmpeg]', data.toString());
  });

  req.on('close', () => {
    ffmpeg.kill('SIGINT');
  });
});

server.listen(PORT, () => {
  console.log(`Listening on http://localhost:${PORT}/`);
});
