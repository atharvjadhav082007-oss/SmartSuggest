import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path';

export default defineConfig({
  plugins: [react()],
  build: {
    outDir: path.resolve(__dirname, '../public'),
    emptyOutDir: true,
  },
  server: {
    port: 3000,
    proxy: {
      '/suggest': 'http://localhost:8080',
      '/game': 'http://localhost:8080',
      '/whatsapp': 'http://localhost:8080',
    },
  },
});
