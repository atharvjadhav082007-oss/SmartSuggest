import React, { useState, useEffect, useRef } from 'react';

const TOKEN_RE = /[A-Za-z0-9']+/g;
const PARAGRAPH_MAX = 5000;

function matchCase(original, fix) {
  return /^[A-Z]/.test(original) ? fix.charAt(0).toUpperCase() + fix.slice(1) : fix;
}

export default function ParagraphView({ onMeasureLatency }) {
  const [text, setText] = useState('');
  const [data, setData] = useState(null);
  const [error, setError] = useState(false);
  const [waMenuOpen, setWaMenuOpen] = useState(false);
  const [flashedWord, setFlashedWord] = useState(null);
  const debounceRef = useRef(null);

  const SAMPLES = [
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
  ];

  useEffect(() => {
    if (!text.trim()) {
      setData(null);
      setError(false);
      return;
    }

    clearTimeout(debounceRef.current);
    debounceRef.current = setTimeout(async () => {
      const start = performance.now();
      try {
        const res = await fetch(`/suggest?mode=paragraph&q=${encodeURIComponent(text)}`);
        if (!res.ok) throw new Error('API failed');
        const json = await res.json();
        onMeasureLatency(`${(performance.now() - start).toFixed(1)} ms`);
        setData(json);
        setError(false);
      } catch (err) {
        console.error(err);
        onMeasureLatency('offline');
        setError(true);
      }
    }, 250);

    return () => clearTimeout(debounceRef.current);
  }, [text]);

  // Compute distinct mistakes
  const distinct = new Map();
  if (data && data.words) {
    data.words.forEach(w => {
      if (w.status === 'misspelled') {
        const key = w.word.toLowerCase();
        if (!distinct.has(key)) {
          distinct.set(key, { suggestions: w.suggestions || [], count: 0 });
        }
        distinct.get(key).count++;
      }
    });
  }

  const replaceWords = (replacements) => {
    const fixed = text.replace(TOKEN_RE, tok => {
      const fix = replacements.get(tok.toLowerCase());
      return fix && /^[A-Za-z]+$/.test(tok) ? matchCase(tok, fix) : tok;
    });
    setText(fixed);
  };

  const handleFixAll = () => {
    if (!data || !data.words) return;
    const map = new Map();
    data.words.forEach(w => {
      if (w.status === 'misspelled' && w.suggestions && w.suggestions.length) {
        map.set(w.word.toLowerCase(), w.suggestions[0].word);
      }
    });
    if (map.size) replaceWords(map);
  };

  const shareWhatsApp = (correctedOnly) => {
    if (!text.trim()) return;
    let msg = '';
    if (correctedOnly) {
      let corrected = text;
      if (data && data.words) {
        const map = new Map();
        data.words.forEach(w => {
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
      if (data && data.words) {
        const mistakes = [];
        const seen = new Set();
        data.words.forEach(w => {
          if (w.status === 'misspelled' && !seen.has(w.word.toLowerCase())) {
            seen.add(w.word.toLowerCase());
            const top = (w.suggestions && w.suggestions.length) ? w.suggestions[0].word : 'no suggestion';
            mistakes.push(`• ${w.word} ➔ ${top}`);
          }
        });
        msg += mistakes.length ? `🔍 Detected Typos & Suggestions:\n${mistakes.join('\n')}` : `✓ No typos found!`;
      }
    }
    if (msg.length > 1500) msg = msg.substring(0, 1490) + '... [truncated]';
    window.open(`https://wa.me/?text=${encodeURIComponent(msg)}`, '_blank', 'noopener,noreferrer');
    setWaMenuOpen(false);
  };

  // Build rendered preview
  const renderPreview = () => {
    if (!data || !data.words) return text;
    const parts = [];
    let last = 0;
    let idx = 0;
    TOKEN_RE.lastIndex = 0;
    let m;
    while ((m = TOKEN_RE.exec(text)) !== null) {
      parts.push(text.slice(last, m.index));
      const info = data.words[idx];
      if (info && info.word === m[0] && info.status === 'misspelled') {
        const wordKey = m[0].toLowerCase();
        parts.push(
          <span
            key={`w-${idx}-${m.index}`}
            className="w-bad"
            title="Click to view corrections"
            onClick={() => {
              setFlashedWord(wordKey);
              setTimeout(() => setFlashedWord(null), 900);
            }}
          >
            {m[0]}
          </span>
        );
      } else {
        parts.push(m[0]);
      }
      last = m.index + m[0].length;
      idx++;
    }
    parts.push(text.slice(last));
    return parts;
  };

  const hasFixable = Array.from(distinct.values()).some(v => v.suggestions.length > 0);

  return (
    <>
      <div className="paragraph-panel">
        <textarea
          id="paragraphInput"
          value={text}
          onChange={(e) => setText(e.target.value)}
          maxLength={PARAGRAPH_MAX}
          rows={6}
          placeholder="Paste or type a paragraph to spell check every word..."
          autoComplete="off"
          spellCheck="false"
          autoFocus
        />
        <div className="paragraph-toolbar">
          <span className="para-count">{text.length} / {PARAGRAPH_MAX}</span>
          <div className="para-actions">
            <button
              type="button"
              className="para-btn"
              disabled={!hasFixable}
              onClick={handleFixAll}
            >
              ✦ Fix all
            </button>
            <div className="wa-dropdown-wrap">
              <button
                type="button"
                className="para-btn wa-btn"
                disabled={!text.trim()}
                onClick={() => setWaMenuOpen(!waMenuOpen)}
                title="Share on WhatsApp"
              >
                <svg viewBox="0 0 24 24" width="15" height="15" fill="currentColor">
                  <path d="M12.04 2C6.58 2 2.13 6.45 2.13 11.91C2.13 13.66 2.59 15.36 3.45 16.86L2.05 22L7.3 20.62C8.75 21.41 10.38 21.83 12.04 21.83C17.5 21.83 21.95 17.38 21.95 11.92C21.95 6.46 17.5 2 12.04 2M12.05 20.16C10.57 20.16 9.12 19.76 7.85 19.01L7.55 18.83L4.43 19.65L5.26 16.61L5.06 16.29C4.24 14.99 3.8 13.47 3.8 11.91C3.8 7.37 7.5 3.67 12.05 3.67C14.25 3.67 16.32 4.53 17.87 6.09C19.42 7.65 20.28 9.72 20.28 11.92C20.28 16.46 16.58 20.16 12.05 20.16M16.57 14.41C16.32 14.28 15.1 13.69 14.88 13.6C14.65 13.52 14.49 13.48 14.32 13.73C14.16 13.98 13.68 14.54 13.54 14.71C13.39 14.88 13.25 14.9 13 14.78C12.75 14.65 11.95 14.39 11 13.55C10.26 12.89 9.76 12.08 9.61 11.83C9.47 11.58 9.6 11.45 9.72 11.33C9.83 11.22 9.97 11.04 10.1 10.89C10.22 10.74 10.27 10.63 10.35 10.47C10.43 10.3 10.39 10.16 10.33 10.03C10.27 9.91 9.78 8.7 9.57 8.21C9.37 7.73 9.17 7.79 9.02 7.78H8.55C8.38 7.78 8.12 7.84 7.9 8.09C7.67 8.33 7.03 8.93 7.03 10.16C7.03 11.39 7.93 12.57 8.05 12.74C8.18 12.9 9.81 15.42 12.3 16.5C12.9 16.75 13.36 16.91 13.72 17.03C14.32 17.21 14.87 17.19 15.3 17.12C15.79 17.05 16.8 16.51 17.01 15.93C17.22 15.35 17.22 14.86 17.15 14.75C17.09 14.65 16.82 14.53 16.57 14.41Z"/>
                </svg>
                Share
              </button>
              {waMenuOpen && (
                <div className="wa-menu open">
                  <button type="button" className="wa-menu-item" onClick={() => shareWhatsApp(true)}>
                    <strong>✦ Share Auto-Corrected</strong>
                    <span>Send clean corrected text</span>
                  </button>
                  <button type="button" className="wa-menu-item" onClick={() => shareWhatsApp(false)}>
                    <strong>📋 Share with Mistakes</strong>
                    <span>Original text + list of typos</span>
                  </button>
                </div>
              )}
            </div>
            <button type="button" className="para-btn ghost" onClick={() => setText('')}>
              Clear
            </button>
          </div>
        </div>
      </div>

      <div className="sample-chips-bar">
        <span className="chips-label">
          <svg viewBox="0 0 24 24" width="12" height="12" fill="none" stroke="currentColor" strokeWidth="2">
            <polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"></polygon>
          </svg>
          Quick Test:
        </span>
        <div className="chips-row">
          {SAMPLES.map((s) => (
            <button key={s.label} type="button" className="sample-chip" onClick={() => setText(s.text)}>
              <span>{s.icon}</span> <span>{s.label}</span>
            </button>
          ))}
        </div>
      </div>

      <div className="results-area">
        {error ? (
          <div className="verdict-card danger">
            <div className="verdict-icon-box">🔌</div>
            <div className="verdict-body">
              <h4>Backend Connection Error</h4>
              <p>Unable to reach C++ server. Ensure <code>app.exe --server</code> is running.</p>
            </div>
          </div>
        ) : !text.trim() || !data ? (
          <div className="empty-state">
            <div className="empty-icon-glow">📝</div>
            <h3>Paragraph Spell Check</h3>
            <p>Paste any article, email, or message. Words are tokenized and checked through C++ Trie & Levenshtein.</p>
          </div>
        ) : (
          <>
            <div className="para-stats">
              <div className="para-stat"><span className="num">{data.totalWords || 0}</span><span className="lbl">Words</span></div>
              <div className={`para-stat ${(data.misspelledCount || 0) > 0 ? 'bad' : 'good'}`}>
                <span className="num">{data.misspelledCount || 0}</span><span className="lbl">Misspelled</span>
              </div>
              <div className="para-stat">
                <span className="num">{(data.totalWords || 0) - (data.checkedWords || 0)}</span><span className="lbl">Skipped</span>
              </div>
            </div>

            <div className="para-preview">{renderPreview()}</div>

            {data.misspelledCount === 0 ? (
              <div className="verdict-card success">
                <div className="verdict-icon-box">✓</div>
                <div className="verdict-body">
                  <h4>No spelling mistakes found</h4>
                  <p>Every checked word exists in the dictionary.</p>
                </div>
              </div>
            ) : (
              <>
                <div className="results-header-bar">
                  <span className="results-count-pill">
                    {distinct.size} Distinct Mistake{distinct.size > 1 ? 's' : ''}
                  </span>
                  <span className="results-badge-meta">ParagraphChecker → SpellChecker</span>
                </div>

                {Array.from(distinct.entries()).map(([badWord, info]) => (
                  <div
                    key={badWord}
                    className={`misspelled-card ${flashedWord === badWord ? 'flash' : ''}`}
                  >
                    <span className="bad-word">{badWord}</span>
                    <span className="times">&times;{info.count}</span>
                    <div className="fix-row">
                      {info.suggestions.length ? (
                        info.suggestions.map(sg => (
                          <button
                            key={sg.word}
                            type="button"
                            className="fix-chip"
                            onClick={() => replaceWords(new Map([[badWord, sg.word]]))}
                          >
                            {sg.word} <small>d={sg.distance}</small>
                          </button>
                        ))
                      ) : (
                        <span className="no-fix">No close matches (&le; 2)</span>
                      )}
                    </div>
                  </div>
                ))}
              </>
            )}
          </>
        )}
      </div>
    </>
  );
}
