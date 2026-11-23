// State Management
let currentMode = 'autocomplete';
let debounceTimer = null;
let activeSelectedIndex = -1;
let currentSuggestions = [];

// DOM Elements
const tabAutocomplete = document.getElementById('tab-autocomplete');
const tabSpellcheck = document.getElementById('tab-spellcheck');
const tabParagraph = document.getElementById('tab-paragraph');
const tabGrammar = document.getElementById('tab-grammar');
const tabGame = document.getElementById('tab-game');
const inputOuter = document.getElementById('inputOuter');
const paragraphPanel = document.getElementById('paragraphPanel');
const paragraphInput = document.getElementById('paragraphInput');
const paraCount = document.getElementById('paraCount');
const fixAllBtn = document.getElementById('fixAllBtn');
const waShareBtn = document.getElementById('waShareBtn');
const waMenu = document.getElementById('waMenu');
const waShareCorrected = document.getElementById('waShareCorrected');
const waShareReport = document.getElementById('waShareReport');
const paraClearBtn = document.getElementById('paraClearBtn');

// Grammar DOM Elements
const grammarPanel = document.getElementById('grammarPanel');
const grammarInput = document.getElementById('grammarInput');
const grammarCount = document.getElementById('grammarCount');
const grammarFixAllBtn = document.getElementById('grammarFixAllBtn');
const grammarClearBtn = document.getElementById('grammarClearBtn');

// Game DOM Elements
const gamePanel = document.getElementById('gamePanel');
const gameLevelSelect = document.getElementById('gameLevelSelect');
const newGameBtn = document.getElementById('newGameBtn');
const gameTimerEl = document.getElementById('gameTimer');
const scoreVal = document.getElementById('scoreVal');
const streakVal = document.getElementById('streakVal');
const bestStreakVal = document.getElementById('bestStreakVal');
const scrambledWordEl = document.getElementById('scrambledWord');
const gameAnswerInput = document.getElementById('gameAnswerInput');
const submitAnswerBtn = document.getElementById('submitAnswerBtn');
const gameFeedback = document.getElementById('gameFeedback');
const chipsBar = document.getElementById('chipsBar');

const PARAGRAPH_MAX = 5000;
let paragraphTimer = null;
let paragraphSeq = 0;      // guards against out-of-order (stale) responses
let lastParagraphData = null;
let grammarTimer = null;
let grammarSeq = 0;
let lastGrammarData = null;
const queryInput = document.getElementById('queryInput');
const clearBtn = document.getElementById('clearBtn');
const resultsArea = document.getElementById('resultsArea');
const chipsContainer = document.getElementById('chipsContainer');
const latencyBadge = document.getElementById('latencyBadge');
const insClass = document.getElementById('insClass');
const insStore = document.getElementById('insStore');
const insDispatch = document.getElementById('insDispatch');
const kbdHint = document.getElementById('kbdHint');

// Game State
let currentGameSession = null;
let gameCountdown = 30;
let gameInterval = null;
let totalGameScore = 0;
let currentGameStreak = 0;
let bestGameStreak = 0;

// Sample Test Chips per mode
const SAMPLES = {
  autocomplete: [
    { label: 'pro', icon: '⚡' },
    { label: 'comp', icon: '⚡' },
    { label: 'algo', icon: '⚡' },
    { label: 'poly', icon: '⚡' },
    { label: 'str', icon: '⚡' }
  ],
  spellcheck: [
    { label: 'progam', icon: '🔍' },
    { label: 'definately', icon: '🔍' },
    { label: 'recieve', icon: '🔍' },
    { label: 'program', icon: '✓' },
    { label: 'tomorow', icon: '🔍' }
  ],
  paragraph: [
    {
      label: 'Sample with typos',
      icon: '📝',
      text: 'The developer wrote a progam and used the debuger to find the memory. We definately need to recieve the sofware tomorow.'
    },
    {
      label: 'Clean text',
      icon: '✓',
      text: 'The developer wrote a program and used the debugger to find the memory.'
    }
  ],
  grammar: [
    {
      label: 'Subject-Verb & Articles',
      icon: '✍️',
      text: 'he go to school every day. She have an car and a apple.'
    },
    {
      label: 'Common Mistakes',
      icon: '🔍',
      text: 'i could of told you that alot of people would of helped.'
    },
    {
      label: 'Double Words & Pronouns',
      icon: '🔤',
      text: 'me went to the the store yesterday. it was closed'
    }
  ],
  game: []
};

// Initialize Application
document.addEventListener('DOMContentLoaded', () => {
  renderChips();
  setupEventListeners();
  updateInspectorMeta();
  initParticleCanvas();
  addTabRippleEffects();
  queryInput.focus();
});

// ============== PARTICLE BACKGROUND ==============
function initParticleCanvas() {
  const canvas = document.getElementById('particleCanvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  let particles = [];
  const PARTICLE_COUNT = 40;

  function resize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
  }
  resize();
  window.addEventListener('resize', resize);

  class Particle {
    constructor() {
      this.reset();
    }
    reset() {
      this.x = Math.random() * canvas.width;
      this.y = Math.random() * canvas.height;
      this.size = Math.random() * 3 + 1;
      this.speedX = (Math.random() - 0.5) * 0.4;
      this.speedY = (Math.random() - 0.5) * 0.4;
      this.opacity = Math.random() * 0.3 + 0.1;
      this.hue = Math.random() > 0.5 ? 240 : 270; // indigo / violet
    }
    update() {
      this.x += this.speedX;
      this.y += this.speedY;
      if (this.x < 0 || this.x > canvas.width || this.y < 0 || this.y > canvas.height) {
        this.reset();
      }
    }
    draw() {
      ctx.beginPath();
      ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
      ctx.fillStyle = `hsla(${this.hue}, 70%, 60%, ${this.opacity})`;
      ctx.fill();
    }
  }

  for (let i = 0; i < PARTICLE_COUNT; i++) {
    particles.push(new Particle());
  }

  function connectParticles() {
    for (let a = 0; a < particles.length; a++) {
      for (let b = a + 1; b < particles.length; b++) {
        const dx = particles[a].x - particles[b].x;
        const dy = particles[a].y - particles[b].y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist < 120) {
          ctx.beginPath();
          ctx.strokeStyle = `rgba(99, 102, 241, ${0.08 * (1 - dist / 120)})`;
          ctx.lineWidth = 0.5;
          ctx.moveTo(particles[a].x, particles[a].y);
          ctx.lineTo(particles[b].x, particles[b].y);
          ctx.stroke();
        }
      }
    }
  }

  function animate() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    particles.forEach(p => { p.update(); p.draw(); });
    connectParticles();
    requestAnimationFrame(animate);
  }
  animate();
}

// ============== TAB RIPPLE EFFECT ==============
function addTabRippleEffects() {
  document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', function(e) {
      const ripple = document.createElement('span');
      ripple.className = 'ripple';
      const rect = this.getBoundingClientRect();
      const size = Math.max(rect.width, rect.height);
      ripple.style.width = ripple.style.height = size + 'px';
      ripple.style.left = (e.clientX - rect.left - size / 2) + 'px';
      ripple.style.top = (e.clientY - rect.top - size / 2) + 'px';
      this.appendChild(ripple);
      ripple.addEventListener('animationend', () => ripple.remove());
    });
  });
}

// ============== CONFETTI BURST ==============
function burstConfetti(container) {
  const colors = ['#4f46e5', '#7c3aed', '#10b981', '#f59e0b', '#ef4444', '#06b6d4'];
  const rect = container.getBoundingClientRect();
  for (let i = 0; i < 20; i++) {
    const piece = document.createElement('div');
    piece.className = 'confetti-piece';
    piece.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
    piece.style.left = (rect.width / 2 + (Math.random() - 0.5) * 100) + 'px';
    piece.style.top = '20px';
    piece.style.animationDelay = (Math.random() * 0.3) + 's';
    piece.style.animationDuration = (0.8 + Math.random() * 0.6) + 's';
    container.style.position = 'relative';
    container.appendChild(piece);
    piece.addEventListener('animationend', () => piece.remove());
  }
}

function setupEventListeners() {
  // Tab Switching
  tabAutocomplete.addEventListener('click', () => switchTab('autocomplete'));
  tabSpellcheck.addEventListener('click', () => switchTab('spellcheck'));
  tabParagraph.addEventListener('click', () => switchTab('paragraph'));
  tabGrammar.addEventListener('click', () => switchTab('grammar'));
  tabGame.addEventListener('click', () => switchTab('game'));

  // Paragraph textarea: live check (debounced) + counter
  paragraphInput.addEventListener('input', () => {
    paraCount.textContent = `${paragraphInput.value.length} / ${PARAGRAPH_MAX}`;
    updateWhatsAppShareButtons();
    clearTimeout(paragraphTimer);
    paragraphTimer = setTimeout(() => fetchParagraph(paragraphInput.value), 400);
  });

  paraClearBtn.addEventListener('click', () => {
    paragraphInput.value = '';
    paraCount.textContent = `0 / ${PARAGRAPH_MAX}`;
    lastParagraphData = null;
    fixAllBtn.disabled = true;
    updateWhatsAppShareButtons();
    showEmptyState();
    paragraphInput.focus();
  });

  fixAllBtn.addEventListener('click', fixAllParagraph);

  // Grammar textarea: live check (debounced) + counter
  grammarInput.addEventListener('input', () => {
    grammarCount.textContent = `${grammarInput.value.length} / ${PARAGRAPH_MAX}`;
    clearTimeout(grammarTimer);
    grammarTimer = setTimeout(() => fetchGrammar(grammarInput.value), 400);
  });

  grammarClearBtn.addEventListener('click', () => {
    grammarInput.value = '';
    grammarCount.textContent = `0 / ${PARAGRAPH_MAX}`;
    lastGrammarData = null;
    grammarFixAllBtn.disabled = true;
    showEmptyState();
    grammarInput.focus();
  });

  grammarFixAllBtn.addEventListener('click', fixAllGrammar);

  // WhatsApp Share Button & Menu
  waShareBtn.addEventListener('click', (e) => {
    e.stopPropagation();
    if (waShareBtn.disabled) return;
    waMenu.classList.toggle('open');
  });

  document.addEventListener('click', () => {
    waMenu.classList.remove('open');
  });

  waShareCorrected.addEventListener('click', () => {
    shareOnWhatsApp(true);
    waMenu.classList.remove('open');
  });

  waShareReport.addEventListener('click', () => {
    shareOnWhatsApp(false);
    waMenu.classList.remove('open');
  });

  // Game Event Listeners
  newGameBtn.addEventListener('click', startNewGameRound);
  submitAnswerBtn.addEventListener('click', submitGameAnswer);
  gameAnswerInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      submitGameAnswer();
    }
  });

  // Live Input Debounce
  queryInput.addEventListener('input', (e) => {
    const val = e.target.value;
    clearBtn.style.display = val.length > 0 ? 'flex' : 'none';
    kbdHint.textContent = val.length > 0 ? `${val.length}ch` : 'live';

    clearTimeout(debounceTimer);
    debounceTimer = setTimeout(() => {
      fetchSuggestions(val.trim());
    }, 100);
  });

  clearBtn.addEventListener('click', () => {
    queryInput.value = '';
    clearBtn.style.display = 'none';
    kbdHint.textContent = 'live';
    showEmptyState();
    queryInput.focus();
  });

  // Keyboard navigation for suggestions
  queryInput.addEventListener('keydown', handleKeyNavigation);
}

function updateWhatsAppShareButtons() {
  const hasText = paragraphInput.value.trim().length > 0;
  waShareBtn.disabled = !hasText;
}

function shareOnWhatsApp(correctedOnly) {
  const text = paragraphInput.value.trim();
  if (!text) return;

  let msg = '';
  if (correctedOnly) {
    let corrected = text;
    if (lastParagraphData && lastParagraphData.data && lastParagraphData.data.words) {
      const map = new Map();
      lastParagraphData.data.words.forEach(w => {
        if (w.status === 'misspelled' && w.suggestions && w.suggestions.length) {
          map.set(w.word.toLowerCase(), w.suggestions[0].word);
        }
      });
      corrected = text.replace(TOKEN_RE, tok => {
        const fix = map.get(tok.toLowerCase());
        return fix && /^[A-Za-z]+$/.test(tok) ? matchCase(tok, fix) : tok;
      });
    }
    msg = `✨ Auto-Corrected Text:\n\n${corrected}`;
  } else {
    msg = `📝 Original Text:\n${text}\n\n`;
    if (lastParagraphData && lastParagraphData.data && lastParagraphData.data.words) {
      const mistakes = [];
      const seen = new Set();
      lastParagraphData.data.words.forEach(w => {
        if (w.status === 'misspelled' && !seen.has(w.word.toLowerCase())) {
          seen.add(w.word.toLowerCase());
          const top = (w.suggestions && w.suggestions.length) ? w.suggestions[0].word : 'no suggestion';
          mistakes.push(`• ${w.word} ➔ ${top}`);
        }
      });
      if (mistakes.length) {
        msg += `🔍 Detected Typos & Suggestions:\n${mistakes.join('\n')}`;
      } else {
        msg += `✓ No typos found!`;
      }
    }
  }

  // Safe length cut ~1500 chars
  if (msg.length > 1500) {
    msg = msg.substring(0, 1490) + '... [truncated]';
  }

  const shareUrl = `https://wa.me/?text=${encodeURIComponent(msg)}`;
  window.open(shareUrl, '_blank', 'noopener,noreferrer');
}

// Switch Active Mode
function switchTab(mode) {
  currentMode = mode;
  [tabAutocomplete, tabSpellcheck, tabParagraph, tabGrammar, tabGame].forEach(tab => {
    tab.classList.remove('active');
    tab.setAttribute('aria-selected', 'false');
  });

  if (mode === 'autocomplete') {
    tabAutocomplete.classList.add('active');
    tabAutocomplete.setAttribute('aria-selected', 'true');
    inputOuter.style.display = 'block';
    paragraphPanel.style.display = 'none';
    grammarPanel.style.display = 'none';
    gamePanel.style.display = 'none';
    chipsBar.style.display = 'flex';
    resultsArea.style.display = 'flex';
    queryInput.placeholder = "Type a prefix (e.g. 'pro')...";
  } else if (mode === 'spellcheck') {
    tabSpellcheck.classList.add('active');
    tabSpellcheck.setAttribute('aria-selected', 'true');
    inputOuter.style.display = 'block';
    paragraphPanel.style.display = 'none';
    grammarPanel.style.display = 'none';
    gamePanel.style.display = 'none';
    chipsBar.style.display = 'flex';
    resultsArea.style.display = 'flex';
    queryInput.placeholder = "Type a word to verify (e.g. 'definately')...";
  } else if (mode === 'paragraph') {
    tabParagraph.classList.add('active');
    tabParagraph.setAttribute('aria-selected', 'true');
    inputOuter.style.display = 'none';
    paragraphPanel.style.display = 'block';
    grammarPanel.style.display = 'none';
    gamePanel.style.display = 'none';
    chipsBar.style.display = 'flex';
    resultsArea.style.display = 'flex';
  } else if (mode === 'grammar') {
    tabGrammar.classList.add('active');
    tabGrammar.setAttribute('aria-selected', 'true');
    inputOuter.style.display = 'none';
    paragraphPanel.style.display = 'none';
    grammarPanel.style.display = 'block';
    gamePanel.style.display = 'none';
    chipsBar.style.display = 'flex';
    resultsArea.style.display = 'flex';
  } else if (mode === 'game') {
    tabGame.classList.add('active');
    tabGame.setAttribute('aria-selected', 'true');
    inputOuter.style.display = 'none';
    paragraphPanel.style.display = 'none';
    grammarPanel.style.display = 'none';
    gamePanel.style.display = 'block';
    chipsBar.style.display = 'none';
    resultsArea.style.display = 'none';
    if (!currentGameSession) {
      startNewGameRound();
    }
  }

  renderChips();
  updateInspectorMeta();

  if (mode === 'paragraph') {
    if (paragraphInput.value.trim()) {
      fetchParagraph(paragraphInput.value);
    } else {
      showEmptyState();
    }
    paragraphInput.focus();
  } else if (mode === 'grammar') {
    if (grammarInput.value.trim()) {
      fetchGrammar(grammarInput.value);
    } else {
      showEmptyState();
    }
    grammarInput.focus();
  } else if (mode === 'game') {
    gameAnswerInput.focus();
  } else {
    if (queryInput.value.trim()) {
      fetchSuggestions(queryInput.value.trim());
    } else {
      showEmptyState();
    }
    queryInput.focus();
  }
}

// Render dynamic sample chips
function renderChips() {
  chipsContainer.innerHTML = '';
  const chips = SAMPLES[currentMode] || [];
  chips.forEach(item => {
    const chip = document.createElement('button');
    chip.type = 'button';
    chip.className = 'sample-chip';
    chip.innerHTML = `<span>${item.icon}</span> <span>${item.label}</span>`;
    chip.addEventListener('click', () => {
      if (currentMode === 'paragraph') {
        paragraphInput.value = item.text || item.label;
        paraCount.textContent = `${paragraphInput.value.length} / ${PARAGRAPH_MAX}`;
        updateWhatsAppShareButtons();
        fetchParagraph(paragraphInput.value);
      } else if (currentMode === 'grammar') {
        grammarInput.value = item.text || item.label;
        grammarCount.textContent = `${grammarInput.value.length} / ${PARAGRAPH_MAX}`;
        fetchGrammar(grammarInput.value);
      } else {
        queryInput.value = item.label;
        clearBtn.style.display = 'flex';
        kbdHint.textContent = `${item.label.length}ch`;
        fetchSuggestions(item.label);
      }
    });
    chipsContainer.appendChild(chip);
  });
}

function updateInspectorMeta() {
  if (currentMode === 'autocomplete') {
    insClass.textContent = 'Autocomplete';
    insStore.textContent = 'Prefix Trie (RAII)';
    insDispatch.textContent = 'Virtual suggest()';
  } else if (currentMode === 'spellcheck') {
    insClass.textContent = 'SpellChecker';
    insStore.textContent = 'Trie + Levenshtein';
    insDispatch.textContent = 'Virtual suggest()';
  } else if (currentMode === 'paragraph') {
    insClass.textContent = 'ParagraphChecker';
    insStore.textContent = 'HAS-A SpellChecker';
    insDispatch.textContent = 'suggest() -> mistakes';
  } else if (currentMode === 'grammar') {
    insClass.textContent = 'GrammarChecker';
    insStore.textContent = 'Rule-Based Engine';
    insDispatch.textContent = 'suggest() -> issues';
  } else if (currentMode === 'game') {
    insClass.textContent = 'SpellingGame';
    insStore.textContent = 'Levenshtein Scoring';
    insDispatch.textContent = 'Virtual suggest()';
  }
}

// ---------------- Game Mode Logic ----------------
async function startNewGameRound() {
  clearInterval(gameInterval);
  gameFeedback.textContent = '';
  gameFeedback.className = 'game-feedback';
  gameAnswerInput.value = '';
  gameAnswerInput.disabled = false;
  submitAnswerBtn.disabled = false;
  gameCountdown = 30;
  gameTimerEl.textContent = `${gameCountdown}s`;
  gameTimerEl.style.color = '#059669';
  gameTimerEl.classList.remove('danger');

  const level = gameLevelSelect.value;
  try {
    const res = await fetch(`/game/new?level=${level}`);
    if (!res.ok) throw new Error('Failed to start game');
    const data = await res.json();
    currentGameSession = data;
    scrambledWordEl.textContent = data.scrambled;
    scrambledWordEl.classList.add('flash');
    setTimeout(() => scrambledWordEl.classList.remove('flash'), 500);

    // Start 30s countdown timer
    gameInterval = setInterval(() => {
      gameCountdown--;
      gameTimerEl.textContent = `${gameCountdown}s`;
      if (gameCountdown <= 10) {
        gameTimerEl.style.color = '#dc2626';
        gameTimerEl.classList.add('danger');
      }
      if (gameCountdown <= 0) {
        clearInterval(gameInterval);
        handleGameTimeout();
      }
    }, 1000);

    gameAnswerInput.focus();
  } catch (err) {
    console.error('Game start error:', err);
    scrambledWordEl.textContent = 'ERROR';
  }
}

function handleGameTimeout() {
  gameAnswerInput.disabled = true;
  submitAnswerBtn.disabled = true;
  currentGameStreak = 0;
  streakVal.textContent = `🔥 0`;
  scrambledWordEl.classList.add('shake');
  setTimeout(() => scrambledWordEl.classList.remove('shake'), 700);
  gameFeedback.textContent = `⏱️ Time's up! Press "New Word" to play again.`;
  gameFeedback.className = 'game-feedback wrong';
}

async function submitGameAnswer() {
  if (!currentGameSession || gameCountdown <= 0) return;
  const answer = gameAnswerInput.value.trim();
  if (!answer) return;

  clearInterval(gameInterval);
  gameAnswerInput.disabled = true;
  submitAnswerBtn.disabled = true;

  try {
    const url = `/game/answer?id=${encodeURIComponent(currentGameSession.id)}&a=${encodeURIComponent(answer)}`;
    const res = await fetch(url);
    if (!res.ok) throw new Error('Answer evaluation failed');
    const data = await res.json();

    totalGameScore = data.totalScore;
    currentGameStreak = data.streak;
    if (currentGameStreak > bestGameStreak) {
      bestGameStreak = currentGameStreak;
    }

    scoreVal.textContent = totalGameScore;
    streakVal.textContent = `🔥 ${currentGameStreak}`;
    bestStreakVal.textContent = bestGameStreak;

    // Pop animation on score cards
    document.querySelectorAll('.score-card').forEach(card => {
      card.classList.remove('pop');
      void card.offsetWidth; // force reflow
      card.classList.add('pop');
    });

    if (data.correct) {
      gameFeedback.textContent = `🎉 ${data.message} (+${data.points} pts) [Target: ${data.targetWord}]`;
      gameFeedback.className = 'game-feedback correct';
      burstConfetti(document.querySelector('.game-challenge-area'));
    } else if (data.distance <= 2) {
      gameFeedback.textContent = `⚡ ${data.message} (+${data.points} pts) [Target: ${data.targetWord}]`;
      gameFeedback.className = 'game-feedback close';
    } else {
      gameFeedback.textContent = `❌ ${data.message} [Target: ${data.targetWord}]`;
      gameFeedback.className = 'game-feedback wrong';
    }

    // Auto load next challenge after 2.5s
    setTimeout(() => {
      startNewGameRound();
    }, 2500);

  } catch (err) {
    console.error('Submit answer error:', err);
    gameFeedback.textContent = 'Error connecting to game server.';
    gameFeedback.className = 'game-feedback wrong';
  }
}

// Fetch suggestions from C++ HTTP server
async function fetchSuggestions(query) {
  if (!query) {
    showEmptyState();
    return;
  }

  const startTime = performance.now();
  try {
    const url = `/suggest?mode=${currentMode}&q=${encodeURIComponent(query)}`;
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Server returned HTTP ${response.status}`);
    }
    const data = await response.json();
    const elapsed = (performance.now() - startTime).toFixed(1);
    latencyBadge.textContent = `${elapsed} ms`;
    latencyBadge.style.color = '#059669';

    if (currentMode === 'autocomplete') {
      renderAutocompleteResults(data, query);
    } else if (currentMode === 'spellcheck') {
      renderSpellCheckResults(data, query);
    }
  } catch (err) {
    console.error('Fetch error:', err);
    latencyBadge.textContent = 'offline';
    latencyBadge.style.color = '#dc2626';
    resultsArea.innerHTML = `
      <div class="verdict-card danger">
        <div class="verdict-icon-box">🔌</div>
        <div class="verdict-body">
          <h4>Backend Connection Error</h4>
          <p>Unable to reach the C++ server. Ensure <code>app.exe --server</code> is running.</p>
        </div>
      </div>
    `;
  }
}

function showEmptyState() {
  activeSelectedIndex = -1;
  currentSuggestions = [];
  resultsArea.innerHTML = `
    <div class="empty-state">
      <div class="empty-icon-glow">⚡</div>
      <h3>Ready for input</h3>
      <p>Type letters above to see live C++ polymorphic suggestions in real time.</p>
    </div>
  `;
}

function escapeHtml(str) {
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

function renderAutocompleteResults(data, prefix) {
  const suggestions = data.suggestions || [];
  currentSuggestions = suggestions;
  activeSelectedIndex = -1;

  if (suggestions.length === 0) {
    resultsArea.innerHTML = `
      <div class="empty-state">
        <div class="empty-icon-glow">🔍</div>
        <h3>No completions found</h3>
        <p>No words in the Trie start with prefix "<strong>${escapeHtml(prefix)}</strong>".</p>
      </div>
    `;
    return;
  }

  let html = `
    <div class="results-header-bar">
      <span class="results-count-pill">${suggestions.length} Result${suggestions.length > 1 ? 's' : ''}</span>
      <span class="results-badge-meta">Prefix Trie (DFS Search)</span>
    </div>
    <ul class="suggestions-grid" id="suggestionsList">
  `;

  const lowerPrefix = prefix.toLowerCase();
  suggestions.forEach((word, idx) => {
    let wordHtml = escapeHtml(word);
    if (word.toLowerCase().startsWith(lowerPrefix)) {
      const matchPart = word.substring(0, prefix.length);
      const restPart = word.substring(prefix.length);
      wordHtml = `<span class="prefix-match">${escapeHtml(matchPart)}</span>${escapeHtml(restPart)}`;
    }

    html += `
      <li class="suggestion-card" data-index="${idx}" data-word="${escapeHtml(word)}">
        <div class="word-content">
          <span class="word-index">#${idx + 1}</span>
          <span class="word-text">${wordHtml}</span>
        </div>
        <div class="card-action">
          <span class="action-tag">Complete</span>
          <span class="action-arrow">↵</span>
        </div>
      </li>
    `;
  });

  html += `</ul>`;
  resultsArea.innerHTML = html;

  resultsArea.querySelectorAll('.suggestion-card').forEach(item => {
    item.addEventListener('click', () => {
      const selectedWord = item.getAttribute('data-word');
      queryInput.value = selectedWord;
      kbdHint.textContent = `${selectedWord.length}ch`;
      queryInput.focus();
      fetchSuggestions(selectedWord);
    });
  });
}

function renderSpellCheckResults(data, query) {
  const isCorrect = data.isCorrect === true;
  const suggestions = data.suggestions || [];
  activeSelectedIndex = -1;

  if (isCorrect) {
    resultsArea.innerHTML = `
      <div class="verdict-card success">
        <div class="verdict-icon-box">✓</div>
        <div class="verdict-body">
          <h4>Spelled Correctly!</h4>
          <p>"<strong>${escapeHtml(query)}</strong>" is verified and exists in the dictionary.</p>
        </div>
      </div>
      <div class="empty-state" style="min-height: 120px;">
        <p style="color: var(--text-dim); font-size: 0.82rem;">
          ✦ Polymorphic <code>SpellChecker::suggest()</code> returned 0 items (word is already correct).
        </p>
      </div>
    `;
    return;
  }

  let html = `
    <div class="verdict-card warning">
      <div class="verdict-icon-box">✗</div>
      <div class="verdict-body">
        <h4>Spelling Not Found</h4>
        <p>"<strong>${escapeHtml(query)}</strong>" is not in the dictionary. Did you mean:</p>
      </div>
    </div>
  `;

  if (suggestions.length === 0) {
    html += `
      <div class="empty-state" style="min-height: 120px;">
        <div class="empty-icon-glow">❓</div>
        <h3>No close matches</h3>
        <p>No dictionary words found within Levenshtein edit distance &le; 2.</p>
      </div>
    `;
  } else {
    html += `
      <div class="results-header-bar">
        <span class="results-count-pill">${suggestions.length} Correction${suggestions.length > 1 ? 's' : ''}</span>
        <span class="results-badge-meta">Levenshtein Edit Distance</span>
      </div>
      <ul class="suggestions-grid">
    `;

    suggestions.forEach((item, idx) => {
      const word = item.word;
      const dist = item.distance;
      const distClass = dist === 1 ? 'dist-1' : 'dist-2';

      html += `
        <li class="suggestion-card" data-word="${escapeHtml(word)}">
          <div class="word-content">
            <span class="word-index">#${idx + 1}</span>
            <span class="word-text">${escapeHtml(word)}</span>
          </div>
          <div class="card-action">
            <span class="dist-pill ${distClass}">edit distance: ${dist}</span>
            <span class="action-arrow">↵</span>
          </div>
        </li>
      `;
    });

    html += `</ul>`;
  }

  resultsArea.innerHTML = html;

  resultsArea.querySelectorAll('.suggestion-card').forEach(item => {
    item.addEventListener('click', () => {
      const selectedWord = item.getAttribute('data-word');
      queryInput.value = selectedWord;
      kbdHint.textContent = `${selectedWord.length}ch`;
      fetchSuggestions(selectedWord);
      queryInput.focus();
    });
  });
}

// ---------------- Paragraph mode ----------------
const TOKEN_RE = /[A-Za-z0-9']+/g;

function matchCase(original, fix) {
  return /^[A-Z]/.test(original) ? fix.charAt(0).toUpperCase() + fix.slice(1) : fix;
}

async function fetchParagraph(text) {
  const mySeq = ++paragraphSeq;

  if (!text.trim()) {
    lastParagraphData = null;
    fixAllBtn.disabled = true;
    updateWhatsAppShareButtons();
    showEmptyState();
    return;
  }

  const startTime = performance.now();
  try {
    const url = `/suggest?mode=paragraph&q=${encodeURIComponent(text)}`;
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Server returned HTTP ${response.status}`);
    }
    const data = await response.json();
    if (mySeq !== paragraphSeq) return;

    const elapsed = (performance.now() - startTime).toFixed(1);
    latencyBadge.textContent = `${elapsed} ms`;
    latencyBadge.style.color = '#059669';

    lastParagraphData = { text, data };
    updateWhatsAppShareButtons();
    renderParagraphResults(text, data);
  } catch (err) {
    if (mySeq !== paragraphSeq) return;
    console.error('Paragraph fetch error:', err);
    latencyBadge.textContent = 'offline';
    latencyBadge.style.color = '#dc2626';
    resultsArea.innerHTML = `
      <div class="verdict-card danger">
        <div class="verdict-icon-box">🔌</div>
        <div class="verdict-body">
          <h4>Backend Connection Error</h4>
          <p>Unable to reach the C++ server. Verify that <code>app.exe --server</code> is running.</p>
        </div>
      </div>
    `;
  }
}

function renderParagraphResults(text, data) {
  const words = data.words || [];

  let preview = '';
  let last = 0;
  let idx = 0;
  const distinct = new Map();
  TOKEN_RE.lastIndex = 0;
  let m;
  while ((m = TOKEN_RE.exec(text)) !== null) {
    preview += escapeHtml(text.slice(last, m.index));
    const info = words[idx];
    if (info && info.word === m[0] && info.status === 'misspelled') {
      const key = m[0].toLowerCase();
      if (!distinct.has(key)) {
        distinct.set(key, { suggestions: info.suggestions || [], count: 0 });
      }
      distinct.get(key).count++;
      preview += `<span class="w-bad" data-word="${escapeHtml(key)}" title="Click to see fixes">${escapeHtml(m[0])}</span>`;
    } else {
      preview += escapeHtml(m[0]);
    }
    last = m.index + m[0].length;
    idx++;
  }
  preview += escapeHtml(text.slice(last));

  const misspelled = data.misspelledCount || 0;
  const skipped = (data.totalWords || 0) - (data.checkedWords || 0);

  let html = `
    <div class="para-stats">
      <div class="para-stat"><span class="num">${data.totalWords || 0}</span><span class="lbl">Words</span></div>
      <div class="para-stat ${misspelled ? 'bad' : 'good'}"><span class="num">${misspelled}</span><span class="lbl">Misspelled</span></div>
      <div class="para-stat"><span class="num">${skipped}</span><span class="lbl">Skipped</span></div>
    </div>
    <div class="para-preview">${preview}</div>
  `;

  if (data.truncated) {
    html += `<p class="para-note">Only the first ${PARAGRAPH_MAX} characters were checked.</p>`;
  }

  if (misspelled === 0) {
    html += `
      <div class="verdict-card success">
        <div class="verdict-icon-box">✓</div>
        <div class="verdict-body">
          <h4>No spelling mistakes found</h4>
          <p>Every checked word exists in the dictionary. Numbers and contractions (e.g. <code>don't</code>) are skipped by design.</p>
        </div>
      </div>
    `;
    fixAllBtn.disabled = true;
  } else {
    html += `
      <div class="results-header-bar">
        <span class="results-count-pill">${distinct.size} Distinct Mistake${distinct.size > 1 ? 's' : ''}</span>
        <span class="results-badge-meta">ParagraphChecker → SpellChecker</span>
      </div>
    `;
    distinct.forEach((info, word) => {
      const chips = info.suggestions.length
        ? info.suggestions.map(sg =>
            `<button type="button" class="fix-chip" data-bad="${escapeHtml(word)}" data-fix="${escapeHtml(sg.word)}">${escapeHtml(sg.word)}<small>d=${sg.distance}</small></button>`
          ).join('')
        : `<span class="no-fix">No close matches (edit distance &le; 2)</span>`;
      html += `
        <div class="misspelled-card" data-word="${escapeHtml(word)}">
          <span class="bad-word">${escapeHtml(word)}</span>
          <span class="times">×${info.count}</span>
          <div class="fix-row">${chips}</div>
        </div>
      `;
    });
    fixAllBtn.disabled = !Array.from(distinct.values()).some(v => v.suggestions.length > 0);
  }

  resultsArea.innerHTML = html;

  resultsArea.querySelectorAll('.fix-chip').forEach(chip => {
    chip.addEventListener('click', () => {
      replaceWords(new Map([[chip.dataset.bad, chip.dataset.fix]]));
    });
  });

  resultsArea.querySelectorAll('.w-bad').forEach(span => {
    span.addEventListener('click', () => {
      const card = Array.from(resultsArea.querySelectorAll('.misspelled-card'))
        .find(c => c.dataset.word === span.dataset.word);
      if (!card) return;
      card.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
      card.classList.add('flash');
      setTimeout(() => card.classList.remove('flash'), 900);
    });
  });
}

function replaceWords(map) {
  const fixed = paragraphInput.value.replace(TOKEN_RE, tok => {
    const fix = map.get(tok.toLowerCase());
    return fix && /^[A-Za-z]+$/.test(tok) ? matchCase(tok, fix) : tok;
  });
  paragraphInput.value = fixed;
  paraCount.textContent = `${fixed.length} / ${PARAGRAPH_MAX}`;
  updateWhatsAppShareButtons();
  fetchParagraph(fixed);
}

function fixAllParagraph() {
  if (!lastParagraphData) return;
  const map = new Map();
  (lastParagraphData.data.words || []).forEach(w => {
    if (w.status === 'misspelled' && w.suggestions && w.suggestions.length) {
      map.set(w.word.toLowerCase(), w.suggestions[0].word);
    }
  });
  if (map.size) replaceWords(map);
}

// ---------------- Grammar mode ----------------
async function fetchGrammar(text) {
  const mySeq = ++grammarSeq;

  if (!text.trim()) {
    lastGrammarData = null;
    grammarFixAllBtn.disabled = true;
    showEmptyState();
    return;
  }

  const startTime = performance.now();
  try {
    const url = `/suggest?mode=grammar&q=${encodeURIComponent(text)}`;
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Server returned HTTP ${response.status}`);
    }
    const data = await response.json();
    if (mySeq !== grammarSeq) return;

    const elapsed = (performance.now() - startTime).toFixed(1);
    latencyBadge.textContent = `${elapsed} ms`;
    latencyBadge.style.color = '#059669';

    lastGrammarData = { text, data };
    renderGrammarResults(text, data);
  } catch (err) {
    if (mySeq !== grammarSeq) return;
    console.error('Grammar fetch error:', err);
    latencyBadge.textContent = 'offline';
    latencyBadge.style.color = '#dc2626';
    resultsArea.innerHTML = `
      <div class="verdict-card danger">
        <div class="verdict-icon-box">🔌</div>
        <div class="verdict-body">
          <h4>Backend Connection Error</h4>
          <p>Unable to reach the C++ server. Verify that <code>app.exe --server</code> is running.</p>
        </div>
      </div>
    `;
  }
}

function renderGrammarResults(text, data) {
  const issues = data.issues || [];
  const totalWords = data.totalWords || 0;
  const sentenceCount = data.sentenceCount || 0;

  let errorCount = 0;
  let warnCount = 0;
  let styleCount = 0;

  issues.forEach(iss => {
    if (iss.severity === 'error') errorCount++;
    else if (iss.severity === 'warning') warnCount++;
    else styleCount++;
  });

  // Build highlighted preview
  let preview = '';
  let lastPos = 0;

  // Ensure issues are sorted by position
  const sorted = [...issues].sort((a, b) => a.position - b.position);

  sorted.forEach((iss, idx) => {
    if (iss.position > lastPos) {
      preview += escapeHtml(text.substring(lastPos, iss.position));
    }
    const chunk = text.substr(iss.position, iss.length || 1);
    const cls = iss.severity === 'error' ? 'g-error' : (iss.severity === 'warning' ? 'g-warning' : 'g-style');
    preview += `<span class="${cls}" data-issue-idx="${idx}" title="${escapeHtml(iss.message)}">${escapeHtml(chunk || '‸')}</span>`;
    lastPos = iss.position + (iss.length || 0);
  });
  if (lastPos < text.length) {
    preview += escapeHtml(text.substring(lastPos));
  }

  let html = `
    <div class="grammar-stats">
      <div class="grammar-stat"><span class="num">${totalWords}</span><span class="lbl">Words</span></div>
      <div class="grammar-stat"><span class="num">${sentenceCount}</span><span class="lbl">Sentences</span></div>
      <div class="grammar-stat ${errorCount ? 'errors' : 'ok'}"><span class="num">${errorCount}</span><span class="lbl">Errors</span></div>
      <div class="grammar-stat ${warnCount ? 'warnings' : 'ok'}"><span class="num">${warnCount}</span><span class="lbl">Warnings</span></div>
    </div>
    <div class="para-preview">${preview}</div>
  `;

  if (issues.length === 0) {
    html += `
      <div class="verdict-card success">
        <div class="verdict-icon-box">✓</div>
        <div class="verdict-body">
          <h4>No grammar issues detected!</h4>
          <p>Subject-verb agreement, article usage, capitalization, and punctuation all look standard.</p>
        </div>
      </div>
    `;
    grammarFixAllBtn.disabled = true;
  } else {
    grammarFixAllBtn.disabled = false;
    html += `
      <div class="results-header-bar">
        <span class="results-count-pill">${issues.length} Suggestion${issues.length > 1 ? 's' : ''}</span>
        <span class="results-badge-meta">GrammarChecker (Rule-Based C++ Engine)</span>
      </div>
      <div class="grammar-issues-list">
    `;

    sorted.forEach((iss, idx) => {
      const sevClass = `severity-${iss.severity}`;
      const badgeClass = iss.severity;
      html += `
        <div class="grammar-issue-card ${sevClass}" data-issue-idx="${idx}">
          <span class="grammar-severity-badge ${badgeClass}">${escapeHtml(iss.severity)}</span>
          <div class="grammar-issue-body">
            <div class="grammar-issue-rule">${escapeHtml(iss.rule)}</div>
            <div class="grammar-issue-msg">${escapeHtml(iss.message)}</div>
            <div class="grammar-issue-fix">
              ${iss.original ? `<span class="grammar-original">${escapeHtml(iss.original)}</span><span class="grammar-arrow">➔</span>` : ''}
              <button type="button" class="grammar-suggestion-btn" data-fix-idx="${idx}">
                ✦ ${escapeHtml(iss.suggestion || 'Remove')}
              </button>
            </div>
          </div>
        </div>
      `;
    });

    html += `</div>`;
  }

  resultsArea.innerHTML = html;

  // Event handlers for individual issue fixes
  resultsArea.querySelectorAll('.grammar-suggestion-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const idx = parseInt(btn.dataset.fixIdx, 10);
      const targetIssue = sorted[idx];
      if (targetIssue) {
        applySingleGrammarFix(targetIssue);
      }
    });
  });

  // Event handlers for preview click -> scroll to card
  resultsArea.querySelectorAll('[data-issue-idx]').forEach(el => {
    if (el.tagName.toLowerCase() === 'span') {
      el.addEventListener('click', () => {
        const idx = el.dataset.issueIdx;
        const card = resultsArea.querySelector(`.grammar-issue-card[data-issue-idx="${idx}"]`);
        if (card) {
          card.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
          card.classList.add('flash');
          setTimeout(() => card.classList.remove('flash'), 900);
        }
      });
    }
  });
}

function applySingleGrammarFix(issue) {
  const current = grammarInput.value;
  if (issue.position <= current.length) {
    const updated = current.substring(0, issue.position) + issue.suggestion + current.substring(issue.position + issue.length);
    grammarInput.value = updated;
    grammarCount.textContent = `${updated.length} / ${PARAGRAPH_MAX}`;
    fetchGrammar(updated);
  }
}

function fixAllGrammar() {
  if (!lastGrammarData || !lastGrammarData.data) return;
  if (lastGrammarData.data.corrected) {
    grammarInput.value = lastGrammarData.data.corrected;
    grammarCount.textContent = `${lastGrammarData.data.corrected.length} / ${PARAGRAPH_MAX}`;
    fetchGrammar(lastGrammarData.data.corrected);
  }
}

function handleKeyNavigation(e) {
  const items = resultsArea.querySelectorAll('.suggestion-card');
  if (!items.length) return;

  if (e.key === 'ArrowDown') {
    e.preventDefault();
    activeSelectedIndex = Math.min(activeSelectedIndex + 1, items.length - 1);
    updateSelectedCard(items);
  } else if (e.key === 'ArrowUp') {
    e.preventDefault();
    activeSelectedIndex = Math.max(activeSelectedIndex - 1, 0);
    updateSelectedCard(items);
  } else if (e.key === 'Enter') {
    if (activeSelectedIndex >= 0 && activeSelectedIndex < items.length) {
      e.preventDefault();
      items[activeSelectedIndex].click();
    }
  }
}

function updateSelectedCard(items) {
  items.forEach((item, idx) => {
    if (idx === activeSelectedIndex) {
      item.classList.add('selected');
      item.scrollIntoView({ block: 'nearest', behavior: 'smooth' });
    } else {
      item.classList.remove('selected');
    }
  });
}
