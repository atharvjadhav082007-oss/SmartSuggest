// Centralized API configuration for local and cloud deployments (Vercel / Netlify)
export const API_BASE = (import.meta.env.VITE_API_BASE_URL || '').replace(/\/$/, '');

export async function fetchSuggest(mode, query) {
  const url = `${API_BASE}/suggest?mode=${mode}&q=${encodeURIComponent(query)}`;
  return fetch(url);
}

export async function fetchNewGame(level) {
  const url = `${API_BASE}/game/new?level=${level}`;
  return fetch(url);
}

export async function submitGameAnswer(id, answer) {
  const url = `${API_BASE}/game/answer?id=${encodeURIComponent(id)}&a=${encodeURIComponent(answer)}`;
  return fetch(url);
}
