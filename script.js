const input = document.getElementById('input');
const latex = document.getElementById('latex');
const csv = document.getElementById('csv');
const decimals = document.getElementById('decimals');
const sigFigs = document.getElementById('sig-figs');

ConvertModule().then(M => {
  const call = (ptr) => { const s = M.UTF8ToString(ptr); M._free(ptr); return s; };
  const genLatex = M.cwrap('gen_latex', 'number', ['string']);
  const genCsv = M.cwrap('gen_csv', 'number', ['string']);
  const genLatexRounded = M.cwrap('gen_latex_rounded', 'number', ['string', 'number']);
  const genCsvRounded = M.cwrap('gen_csv_rounded', 'number', ['string', 'number']);
  const genLatexSigFigs = M.cwrap('gen_latex_sig_figs', 'number', ['string', 'number']);
  const genCsvSigFigs = M.cwrap('gen_csv_sig_figs', 'number', ['string', 'number']);
  
  const getRoundMode = () => {
    const selected = document.querySelector('input[name="round-mode"]:checked');
    return selected ? selected.value : 'none';
  };
  
  document.getElementById('latex-btn').onclick = () => {
    const data = input.value.trim();
    if (data) {
      const mode = getRoundMode();
      if (mode === 'decimal') {
        latex.value = call(genLatexRounded(data, parseInt(decimals.value) || 0));
      } else if (mode === 'sig-figs') {
        latex.value = call(genLatexSigFigs(data, parseInt(sigFigs.value) || 1));
      } else {
        latex.value = call(genLatex(data));
      }
    }
  };
  
  document.getElementById('csv-btn').onclick = () => {
    const data = input.value.trim();
    if (data) {
      const mode = getRoundMode();
      if (mode === 'decimal') {
        csv.value = call(genCsvRounded(data, parseInt(decimals.value) || 0));
      } else if (mode === 'sig-figs') {
        csv.value = call(genCsvSigFigs(data, parseInt(sigFigs.value) || 1));
      } else {
        csv.value = call(genCsv(data));
      }
    }
  };
});