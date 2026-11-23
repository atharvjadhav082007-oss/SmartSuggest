import React, { useState, useEffect, useRef } from 'react';

export default function GameView() {
  const [level, setLevel] = useState(1);
  const [session, setSession] = useState(null);
  const [answer, setAnswer] = useState('');
  const [score, setScore] = useState(0);
  const [streak, setStreak] = useState(0);
  const [bestStreak, setBestStreak] = useState(0);
  const [countdown, setCountdown] = useState(30);
  const [feedback, setFeedback] = useState(null);
  const [isAnswered, setIsAnswered] = useState(false);
  const [shaking, setShaking] = useState(false);
  const [flashingScrambled, setFlashingScrambled] = useState(false);
  const [cardsPopping, setCardsPopping] = useState(false);
  const timerRef = useRef(null);
  const challengeAreaRef = useRef(null);

  const burstConfetti = () => {
    const container = challengeAreaRef.current;
    if (!container) return;
    const colors = ['#4f46e5', '#7c3aed', '#10b981', '#f59e0b', '#ef4444', '#06b6d4'];
    const rect = container.getBoundingClientRect();
    for (let i = 0; i < 20; i++) {
      const piece = document.createElement('div');
      piece.className = 'confetti-piece';
      piece.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)];
      piece.style.left = `${rect.width / 2 + (Math.random() - 0.5) * 120}px`;
      piece.style.top = '20px';
      piece.style.animationDelay = `${Math.random() * 0.3}s`;
      piece.style.animationDuration = `${0.8 + Math.random() * 0.6}s`;
      container.appendChild(piece);
      piece.addEventListener('animationend', () => piece.remove());
    }
  };

  const startNewRound = async (customLevel = level) => {
    clearInterval(timerRef.current);
    setFeedback(null);
    setAnswer('');
    setIsAnswered(false);
    setCountdown(30);
    setShaking(false);

    try {
      const res = await fetch(`/game/new?level=${customLevel}`);
      if (!res.ok) throw new Error('Failed to start new game');
      const data = await res.json();
      setSession(data);
      setFlashingScrambled(true);
      setTimeout(() => setFlashingScrambled(false), 500);

      timerRef.current = setInterval(() => {
        setCountdown(prev => {
          if (prev <= 1) {
            clearInterval(timerRef.current);
            setIsAnswered(true);
            setStreak(0);
            setShaking(true);
            setFeedback({
              type: 'wrong',
              message: "⏱️ Time's up! Press \"New Word\" to try again."
            });
            return 0;
          }
          return prev - 1;
        });
      }, 1000);
    } catch (err) {
      console.error(err);
      setSession({ scrambled: 'ERROR' });
    }
  };

  useEffect(() => {
    startNewRound(level);
    return () => clearInterval(timerRef.current);
  }, []);

  const handleSubmit = async (e) => {
    if (e) e.preventDefault();
    if (!session || isAnswered || countdown <= 0 || !answer.trim()) return;

    clearInterval(timerRef.current);
    setIsAnswered(true);

    try {
      const url = `/game/answer?id=${encodeURIComponent(session.id)}&a=${encodeURIComponent(answer.trim())}`;
      const res = await fetch(url);
      if (!res.ok) throw new Error('Submit failed');
      const data = await res.json();

      setScore(data.totalScore);
      setStreak(data.streak);
      setBestStreak(prev => Math.max(prev, data.streak));

      setCardsPopping(true);
      setTimeout(() => setCardsPopping(false), 400);

      if (data.correct) {
        setFeedback({
          type: 'correct',
          message: `🎉 ${data.message} (+${data.points} pts) [Target: ${data.targetWord}]`
        });
        burstConfetti();
      } else if (data.distance <= 2) {
        setFeedback({
          type: 'close',
          message: `⚡ ${data.message} (+${data.points} pts) [Target: ${data.targetWord}]`
        });
      } else {
        setFeedback({
          type: 'wrong',
          message: `❌ ${data.message} [Target: ${data.targetWord}]`
        });
      }

      setTimeout(() => {
        startNewRound(level);
      }, 2500);
    } catch (err) {
      console.error(err);
      setFeedback({ type: 'wrong', message: 'Error checking answer.' });
    }
  };

  return (
    <div className="game-panel">
      <div className="game-controls-bar">
        <div className="game-diff-picker">
          <label htmlFor="gameLevelSelect">Difficulty:</label>
          <select
            id="gameLevelSelect"
            className="game-select"
            value={level}
            onChange={(e) => {
              const newLvl = Number(e.target.value);
              setLevel(newLvl);
              startNewRound(newLvl);
            }}
          >
            <option value="1">Level 1 (Easy: 4-6 chars)</option>
            <option value="2">Level 2 (Medium: 6-9 chars)</option>
            <option value="3">Level 3 (Hard: 8+ chars)</option>
          </select>
        </div>
        <button type="button" className="para-btn" onClick={() => startNewRound(level)}>
          🎮 New Word
        </button>
        <div className="game-timer-box">
          <span className="timer-icon">⏱️</span>
          <span className={`timer-value ${countdown <= 10 ? 'danger' : ''}`} style={{ color: countdown <= 10 ? '#dc2626' : '#059669' }}>
            {countdown}s
          </span>
        </div>
      </div>

      <div className="game-scoreboard">
        <div className={`score-card ${cardsPopping ? 'pop' : ''}`}>
          <span className="score-label">Score</span>
          <span className="score-num">{score}</span>
        </div>
        <div className={`score-card ${cardsPopping ? 'pop' : ''}`}>
          <span className="score-label">Streak</span>
          <span className="score-num streak">🔥 {streak}</span>
        </div>
        <div className={`score-card ${cardsPopping ? 'pop' : ''}`}>
          <span className="score-label">Best Streak</span>
          <span className="score-num">{bestStreak}</span>
        </div>
      </div>

      <div className="game-challenge-area" ref={challengeAreaRef} style={{ position: 'relative' }}>
        <div className="challenge-label">Correct or unscramble this word:</div>
        <div className={`scrambled-display ${flashingScrambled ? 'flash' : ''} ${shaking ? 'shake' : ''}`}>
          {session ? session.scrambled : 'LOADING...'}
        </div>

        <form className="game-input-row" onSubmit={handleSubmit}>
          <input
            type="text"
            id="gameAnswerInput"
            value={answer}
            onChange={(e) => setAnswer(e.target.value)}
            disabled={isAnswered}
            placeholder="Type your answer and press Enter..."
            autoComplete="off"
            spellCheck="false"
            autoFocus
          />
          <button type="submit" className="para-btn" disabled={isAnswered || !answer.trim()}>
            Submit ↵
          </button>
        </form>

        {feedback && (
          <div className={`game-feedback ${feedback.type}`}>
            {feedback.message}
          </div>
        )}
      </div>
    </div>
  );
}
