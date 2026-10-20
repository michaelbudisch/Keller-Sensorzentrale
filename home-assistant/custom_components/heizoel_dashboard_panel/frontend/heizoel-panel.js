/* Keller Sensorzentrale UI engine v1.0.0 — no external dependencies */
const HM_VERSION = '2.3.1';
const HM_FIELDS = [
 ['total','Gesamtbestand (5 Min)','sensor','keller_tank_gesamt_5min|gesamtbestand.*(gesch|geschätzt)|estimated_total_liters'],
 ['single','Einzeltank (5 Min)','sensor','keller_tank_einzeln_5min|tank_einzeln|tank einzeln|tank_liters'],
 ['percent','Füllstand (5 Min)','sensor','keller_tank_fuellstand_5min|fuellstand.*gemess|füllstand gemessener tank'],
 ['distance','Tankabstand (5 Min)','sensor','keller_tank_abstand_5min|abstand.*(median|gegl)|distance_cm'],
 ['rawDistance','Abstand Rohmedian','sensor','abstand.*(median|gegl)|distance_cm'],
 ['wifi','WLAN-Signal','sensor','wlan.signal|signal_dbm'],
 ['tankCount','Tankanzahl','sensor','tankanzahl|tank_count'],
 ['low','Niedrigstand','binary_sensor','fullstand niedrig|füllstand niedrig|low_level'],
 ['fault','Sensorstörung','binary_sensor','sensorstorung|sensorstörung|sensor_fault'],
 ['temp1','Slot 1','sensor','(^|[_ ])slot[_ ]?1$|temp_1$'],
 ['temp2','Slot 2','sensor','(^|[_ ])slot[_ ]?2$|temp_2$'],
 ['temp3','Slot 3','sensor','(^|[_ ])slot[_ ]?3$|temp_3$'],
 ['temp4','Slot 4','sensor','(^|[_ ])slot[_ ]?4$|temp_4$'],
 ['temp5','Slot 5','sensor','(^|[_ ])slot[_ ]?5$|temp_5$'],
 ['temp6','Slot 6','sensor','(^|[_ ])slot[_ ]?6$|temp_6$'],
 ['flow','Heizkreis Vorlauf','sensor','vorlauf'],
 ['return','Heizkreis Rücklauf','sensor','ruecklauf|r.cklauf'],
 ['spread','Heizkreis Spreizung (optional)','sensor','heizung_spreizung|temperaturspreizung'],
 ['relay1','Relais 1','switch','relais[_ ]?1$|relay_1$'],
 ['relay2','Relais 2','switch','relais[_ ]?2$|relay_2$'],
 ['daily','Verbrauch heute','sensor','heizoel_verbrauch_heute|heizöl verbrauch heute'],
 ['weekly','Verbrauch Woche','sensor','heizoel_verbrauch_woche|heizöl verbrauch kalenderwoche'],
 ['monthly','Verbrauch Monat','sensor','heizoel_verbrauch_monat|heizöl verbrauch kalendermonat'],
 ['yearly','Verbrauch Jahr','sensor','heizoel_verbrauch_jahr|heizöl verbrauch kalenderjahr'],
 ['lastFill','Letzte Befüllung','sensor','heizoel_letzte_tankbefuellung'],
 ['fillCount','Befüllungen erkannt','sensor','heizoel_tankbefuellungen'],
 ['verbrauchStatus','Diagnose Verbrauch','sensor','heizoel_verbrauch_diagnose'],
 ['verbrauchAktiv','Auswertung freigegeben','input_boolean','keller_verbrauch_aktiv'],
 ['average','Tagesdurchschnitt','sensor','heizoel_durchschnitt_pro_tag|durchschnittlicher tagesverbrauch'],
 ['range','Restreichweite','sensor','heizoel_restreichweite_tage|geschätzte restreichweite'],
 ['median','Median 6 h','sensor','heizoel_bestand_median_6h|bestand median 6h'],
 ['ref','Referenzbestand','sensor','heizoel_referenzbestand'],
];
const HM_DEFAULTS={
 total:'sensor.keller_tank_gesamt_5min', single:'sensor.keller_tank_einzeln_5min',
 percent:'sensor.keller_tank_fuellstand_5min',distance:'sensor.keller_tank_abstand_5min',
 daily:'sensor.heizoel_verbrauch_heute',weekly:'sensor.heizoel_verbrauch_woche',monthly:'sensor.heizoel_verbrauch_monat',
 yearly:'sensor.heizoel_verbrauch_jahr',lastFill:'sensor.heizoel_letzte_tankbefuellung',fillCount:'sensor.heizoel_tankbefuellungen',
 verbrauchStatus:'sensor.heizoel_verbrauch_diagnose',verbrauchAktiv:'input_boolean.keller_verbrauch_aktiv',
 average:'sensor.heizoel_durchschnitt_pro_tag',range:'sensor.heizoel_restreichweite_tage',
 median:'sensor.heizoel_bestand_median_6h',ref:'sensor.heizoel_referenzbestand',
 rawDistance:'sensor.heizungskeller_sensoren_abstand_geglattet',
 wifi:'sensor.heizungskeller_sensoren_wlan_signal',
 tankCount:'sensor.heizungskeller_sensoren_tankanzahl',
 low:'binary_sensor.heizungskeller_sensoren_fullstand_niedrig',
 fault:'binary_sensor.heizungskeller_sensoren_sensorstorung',
 temp1:'sensor.heizungskeller_sensoren_slot_1',
 temp2:'sensor.heizungskeller_sensoren_slot_2',
 temp3:'sensor.heizungskeller_sensoren_slot_3',
 temp4:'sensor.heizungskeller_sensoren_slot_4',
 temp5:'sensor.heizungskeller_sensoren_slot_5',
 temp6:'sensor.heizungskeller_sensoren_slot_6',
 flow:'sensor.heizungskeller_sensoren_slot_1',
 return:'sensor.heizungskeller_sensoren_slot_2',
 spread:'sensor.heizung_spreizung',
 relay1:'switch.heizungskeller_sensoren_relais_1',
 relay2:'switch.heizungskeller_sensoren_relais_2',
};
const HM_SAVE_KEY = 'keller_sensorzentrale_v2_settings';
const HM_STYLES = `
:host{display:block;box-sizing:border-box;color:var(--primary-text-color,#eef3fb);font-family:var(--paper-font-body1_-_font-family,system-ui,Arial,sans-serif);--hm-accent:#38a6dc;--hm-line:rgba(132,160,199,.22);--hm-bg:var(--card-background-color,#19263b);--hm-sub:rgba(132,160,199,.10);--hm-muted:var(--secondary-text-color,#9eafc5)}
*{box-sizing:border-box} button,select,input{font:inherit}button{cursor:pointer}button:disabled{opacity:.5;cursor:not-allowed} button:focus-visible,input:focus-visible,select:focus-visible{outline:2px solid var(--hm-accent);outline-offset:2px}
.hm-wrap{max-width:1440px;margin:auto;padding:clamp(12px,2.3vw,30px) clamp(12px,2.1vw,28px) 64px;}
.hm-head{display:flex;align-items:center;justify-content:space-between;gap:16px;margin:0 0 22px}.hm-head h1{margin:0;font-size:clamp(23px,2.3vw,33px);font-weight:790;letter-spacing:-.035em}.hm-head p{color:var(--hm-muted);font-size:13px;margin:5px 0 0}
.hm-status{display:inline-flex;align-items:center;gap:7px;padding:8px 12px;font-size:12px;background:rgba(42,151,116,.12);border:1px solid rgba(42,151,116,.25);border-radius:99px;color:#6fd6ab;font-weight:720;white-space:nowrap}.hm-status.bad{color:#f3b37a;background:rgba(218,129,60,.1);border-color:rgba(218,129,60,.3)}
.hm-dot{display:inline-block;width:7px;height:7px;border-radius:50%;background:currentColor}
.hm-tabs{display:flex;flex-wrap:wrap;gap:7px;margin:0 0 18px}.hm-tabs button{background:var(--hm-sub);border:1px solid var(--hm-line);border-radius:10px;padding:10px 15px;font-size:13px;font-weight:680;color:inherit}.hm-tabs button.active{background:#2379c6;color:white;border-color:#388cdf}
.hm-hero{display:grid;grid-template-columns:minmax(0,1.3fr) minmax(275px,1fr);gap:16px}.hm-panel{background:var(--hm-bg);border:1px solid var(--hm-line);border-radius:16px;padding:clamp(16px,2vw,24px);box-shadow:0 1px 10px rgba(0,0,0,.035);min-width:0}.hm-panel h2{margin:0 0 17px;font-size:17px;font-weight:740}.hm-panel h3{font-size:14px;margin:0 0 12px}
.hm-total{font-size:clamp(35px,4vw,58px);font-weight:780;letter-spacing:-.055em;line-height:1.1;margin:10px 0 5px;font-variant-numeric:tabular-nums}.hm-label{font-size:12px;color:var(--hm-muted);font-weight:540}.hm-unit{font-size:17px;font-weight:550;letter-spacing:0}
.hm-split{display:flex;justify-content:space-between;gap:12px;margin-top:18px;align-items:center}.hm-split strong{font-size:15px;font-variant-numeric:tabular-nums}.hm-bar{height:11px;border-radius:99px;overflow:hidden;background:rgba(128,149,175,.24);margin:18px 0 8px}.hm-bar>div{height:100%;background:linear-gradient(90deg,#238eaa,#3ab9a4);border-radius:99px;transition:width .35s}
.hm-meter{display:grid;grid-template-columns:125px 1fr;gap:20px;align-items:center}.hm-ring{width:125px;height:125px;border-radius:50%;display:grid;place-items:center;position:relative;background:conic-gradient(#36b8a9 var(--pct,0%),rgba(148,163,184,.18) 0)}.hm-ring:before{content:'';position:absolute;inset:12px;border-radius:50%;background:var(--hm-bg)}.hm-ring span{position:relative;font-size:22px;font-weight:780;font-variant-numeric:tabular-nums}
.hm-small{font-size:12px;color:var(--hm-muted);line-height:1.55}.hm-mini{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:13px;margin-top:15px}.hm-mini.three{grid-template-columns:repeat(3,minmax(0,1fr))}.hm-tile{background:var(--hm-bg);border:1px solid var(--hm-line);padding:17px;border-radius:14px;min-width:0}.hm-tile .n{font-size:25px;font-weight:750;margin-top:8px;line-height:1.15;font-variant-numeric:tabular-nums}.hm-tile .u{font-size:12px;color:var(--hm-muted);font-weight:500}.hm-row{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-top:12px}.hm-row b{font-size:14px;font-variant-numeric:tabular-nums}
.hm-section{margin-top:17px}.hm-note{background:rgba(56,166,220,.08);border:1px solid rgba(56,166,220,.22);border-radius:12px;padding:12px 15px;color:var(--hm-muted);font-size:12px;line-height:1.6;margin-top:15px}
.hm-charthead{display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:10px;margin-bottom:12px}.hm-charthead h2{margin:0}.hm-chartnav{display:flex;gap:4px;padding:4px;background:var(--hm-sub);border-radius:10px}.hm-chartnav button{padding:6px 10px;border:none;border-radius:7px;background:transparent;color:inherit;font-size:12px;font-weight:650}.hm-chartnav button.active{background:var(--hm-bg);box-shadow:0 1px 4px rgba(0,0,0,.14)}
.hm-graph{width:100%;height:auto;display:block;min-height:170px}.hm-graph text{fill:var(--hm-muted);font:12px system-ui,sans-serif}.hm-gridline{stroke:var(--hm-line);stroke-width:1}.hm-area{fill:rgba(55,169,206,.12)}.hm-line{stroke:var(--hm-accent);stroke-width:2.8;fill:none;stroke-linecap:round;stroke-linejoin:round}
.hm-empty{height:200px;display:grid;place-items:center;text-align:center;color:var(--hm-muted);font-size:13px;padding:22px}.hm-error{color:#eab08a}
.hm-settings{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.hm-field{display:flex;flex-direction:column;gap:7px;min-width:0}.hm-field label{font-size:12px;color:var(--hm-muted)}.hm-field select,.hm-field input{background:var(--hm-sub);border:1px solid var(--hm-line);color:inherit;border-radius:9px;padding:10px 12px;min-width:0;width:100%;font-size:13px}
.hm-btn{padding:10px 14px;border:none;background:#287ed2;border-radius:9px;color:white;font-weight:750;font-size:13px}.hm-btn.secondary{background:var(--hm-sub);border:1px solid var(--hm-line);color:inherit}.hm-toolbar{display:flex;gap:9px;flex-wrap:wrap;align-items:center;margin-top:14px}.hm-legend{display:flex;gap:14px;flex-wrap:wrap;color:var(--hm-muted);font-size:12px;margin-top:7px}.hm-legend i{display:inline-block;width:9px;height:9px;border-radius:2px;background:var(--hm-accent);margin-right:6px}
.hm-card{padding:16px;background:var(--ha-card-background,var(--card-background-color,#19263b));border:1px solid var(--hm-line);border-radius:var(--ha-card-border-radius,14px)}.hm-card .hm-total{font-size:38px}.hm-card .hm-mini{grid-template-columns:repeat(3,minmax(0,1fr));gap:8px}.hm-card .hm-tile{padding:10px}.hm-card .hm-tile .n{font-size:18px}.hm-card .hm-graph{min-height:100px}.hm-card .hm-hero{grid-template-columns:1fr 1fr}.hm-card .hm-ring{height:94px;width:94px}.hm-card .hm-ring:before{inset:9px}.hm-card .hm-meter{grid-template-columns:94px 1fr;gap:10px}.hm-card .hm-ring span{font-size:17px}
.hm-footer{color:var(--hm-muted);font-size:11px;line-height:1.6;margin:20px 0 0}.hm-menu{border:none;background:var(--hm-sub);border-radius:9px;color:inherit;font-size:23px;width:38px;height:38px;display:none;align-items:center;justify-content:center}.hm-loading{color:var(--hm-muted);text-align:center;margin:18px}
@media(max-width:850px){.hm-menu{display:flex}.hm-hero{grid-template-columns:1fr}.hm-mini{grid-template-columns:repeat(2,minmax(0,1fr))}.hm-settings{grid-template-columns:1fr}}
@media(max-width:490px){.hm-head{align-items:flex-start}.hm-total{font-size:36px}.hm-mini{grid-template-columns:repeat(2,minmax(0,1fr))}.hm-mini.three{grid-template-columns:repeat(2,minmax(0,1fr))}.hm-panel{padding:15px}.hm-meter{grid-template-columns:100px 1fr}.hm-ring{width:100px;height:100px}.hm-card .hm-hero{grid-template-columns:1fr}.hm-card .hm-mini{grid-template-columns:repeat(2,minmax(0,1fr))}.hm-tabs button{padding:8px 10px;font-size:12px}}

.hm-sensor-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:10px}
.hm-sensor{display:flex;justify-content:space-between;gap:8px;padding:12px 14px;border-radius:11px;border:1px solid var(--hm-line);background:var(--hm-sub);align-items:center;min-width:0}
.hm-sensor strong{font-variant-numeric:tabular-nums;white-space:nowrap}
.hm-sensor .hm-label{min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.hm-relays{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}
.hm-relay{background:var(--hm-sub);border:1px solid var(--hm-line);border-radius:12px;padding:13px}
.hm-relay .hm-toolbar{margin-top:9px}
.hm-relay button{min-width:71px}.hm-relay button.is-on{background:#147e65}.hm-relay button.is-off{background:#596b83}
.hm-graphkeys{display:flex;flex-wrap:wrap;gap:8px 13px;font-size:12px;color:var(--hm-muted)}
.hm-graphkeys i{display:inline-block;width:9px;height:9px;border-radius:50%;margin-right:5px}
@media(max-width:850px){.hm-sensor-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media(max-width:480px){.hm-relays{grid-template-columns:1fr}.hm-sensor-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}
`;
function hmEscape(value){return String(value??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));}
function hmNumber(raw){if(raw===null||raw===undefined||raw===''||['unknown','unavailable','none','null','NaN'].includes(String(raw).trim().toLowerCase()))return null;const n=Number(String(raw).replace(',','.'));return Number.isFinite(n)?n:null;}
function hmFmt(n,dec=1){return Number.isFinite(n)?n.toLocaleString('de-DE',{minimumFractionDigits:dec,maximumFractionDigits:dec}):'–';}
function hmDate(t,short=false){return new Date(t).toLocaleString('de-DE',short?{hour:'2-digit',minute:'2-digit'}:{day:'2-digit',month:'2-digit'});}
function hmSafeId(x){return typeof x==='string'&&/^(sensor|binary_sensor|input_number|input_boolean|switch)\.[a-z0-9_]+$/.test(x)?x:'';}
function hmAutoMap(hass,provided={}){
 const states=hass?.states||{},keys=Object.keys(states);
 const map={};
 for(const [field,,domain,pattern] of HM_FIELDS){
   const specified=provided[field];
   if(hmSafeId(specified)){map[field]=specified;continue;}
   const pref=HM_DEFAULTS[field];
   if(pref&&states[pref]){map[field]=pref;continue;}
   // Prozentwerte duerfen NICHT durch beliebige %-Sensoren wie Akku-SOC ersetzt werden.
   // Zuerst den Füllstand-Sensor desselben MQTT-Geraetes wie den Gesamtbestand suchen.
   if(field==='percent'){
     const totalId=map.total||'';
     const suffix=totalId.replace(/^sensor\./,'').match(/^(.*?)_(?:gesamtbestand_gesch(?:a|ä)tzt|estimated_total_liters)$/i);
     const root=suffix?suffix[1]+'_':'';
     const valid=keys.filter(k=>{
       const ent=states[k];
       if(!k.startsWith('sensor.')||ent?.attributes?.unit_of_measurement!=='%')return false;
       const name=(ent.attributes?.friendly_name||'')+' '+k;
       return /f(?:ü|u|ue)llstand|fill[_ -]?level|tank[_ -]?percent|percent/i.test(name);
     });
     const ranked=valid.map(id=>{
       const name=((states[id].attributes?.friendly_name||'')+' '+id).toLowerCase();
       const related=!!root&&id.slice(7).startsWith(root);
       const fill=/f(?:ü|u|ue)llstand|fill[_ -]?level/.test(name);
       const measured=/gemess|measur|tank/.test(name);
       const stronglyNamed=fill&&measured;
       // Ohne vernuenftigen Bezug KEINEN fremden Prozentwert anzeigen.
       const score=(related?100:0)+(stronglyNamed?30:0)+(fill?10:0);
       return {id,score};
     }).filter(item=>item.score>=30).sort((a,b)=>b.score-a.score);
     map[field]=ranked[0]?.id||'';
     continue;
   }
   const rx=new RegExp(pattern,'i');
   const matches=keys.filter(k=>k.startsWith(domain+'.')&&(rx.test(k)||rx.test(states[k].attributes?.friendly_name||'')));
   const own=matches.find(k=>/heizungskeller_sensoren|sensorzentrale|heizoelmonitor|heizoel_/i.test(k));
   map[field]=own||(field.startsWith('relay')||field.startsWith('temp')?'':matches[0])||'';
 }
 return map;
}
function hmEntity(hass,map,key){return hass?.states?.[map[key]];}
function hmValue(hass,map,key){return hmNumber(hmEntity(hass,map,key)?.state);}
function hmBinary(hass,map,key){const x=hmEntity(hass,map,key)?.state;return x==='on'?true:x==='off'?false:null;}
function hmStatus(hass,map){
 const fault=hmBinary(hass,map,'fault');
 const raw=hmEntity(hass,map,'rawDistance');
 if(raw&&['unknown','unavailable'].includes(raw.state))return {ok:false,text:'Wemos offline – MQTT-Sensoren nicht verfügbar'};
 if(fault===true)return {ok:false,text:'Tanksensor prüfen'};
 if(!hmEntity(hass,map,'total'))return {ok:false,text:'Gesamtbestand nicht zugeordnet'};
 if(!Number.isFinite(hmValue(hass,map,'total')))return {ok:false,text:'Tankwert noch nicht verfügbar'};
 return {ok:true,text:'Sensorzentrale online'};
}
function hmTile(label,val,unit=''){return `<div class="hm-tile"><div class="hm-label">${hmEscape(label)}</div><div class="n">${hmEscape(val)} ${unit?`<span class="u">${hmEscape(unit)}</span>`:''}</div></div>`;}
function hmStateTile(hass,map,key,label,unit='L',dec=1){return hmTile(label,hmFmt(hmValue(hass,map,key),dec),unit);}
function hmSvg(points){
 const data=points.filter(x=>Number.isFinite(x.v)&&Number.isFinite(x.t));
 if(data.length<2)return `<div class="hm-empty">Noch keine ausreichenden historischen Messwerte vorhanden.<br>Home Assistant benötigt dazu Recorder-Daten.</div>`;
 const w=980,h=270,pad={l:54,r:17,t:18,b:30};let lo=Math.min(...data.map(x=>x.v)),hi=Math.max(...data.map(x=>x.v));let spread=hi-lo;if(spread<1){hi+=1;lo-=1}else{lo-=spread*.08;hi+=spread*.08;}const tMin=data[0].t,tMax=data[data.length-1].t,span=Math.max(1,tMax-tMin);const xx=t=>pad.l+(t-tMin)/span*(w-pad.l-pad.r),yy=v=>pad.t+(hi-v)/(hi-lo)*(h-pad.t-pad.b);
 const coords=data.map(d=>[xx(d.t),yy(d.v)]),line=coords.map(([x,y],i)=>`${i?'L':'M'}${x.toFixed(1)} ${y.toFixed(1)}`).join(' '),area=line+` L${coords[coords.length-1][0].toFixed(1)} ${h-pad.b} L${coords[0][0].toFixed(1)} ${h-pad.b} Z`;
 const lines=Array.from({length:4},(_,i)=>{const v=lo+(hi-lo)*i/3,y=yy(v);return `<line class="hm-gridline" x1="${pad.l}" x2="${w-pad.r}" y1="${y}" y2="${y}"/><text x="${pad.l-8}" y="${y+4}" text-anchor="end">${hmFmt(v,0)}</text>`}).join('');const dates=[0,.5,1].map(fr=>{const t=tMin+(tMax-tMin)*fr,x=xx(t);return `<text x="${x.toFixed(1)}" y="${h-8}" text-anchor="middle">${hmDate(t,span<=86400000)}</text>`}).join('');
 return `<svg class="hm-graph" viewBox="0 0 ${w} ${h}" role="img" aria-label="Ölbestand Verlauf in Litern"><title>Ölbestand in Litern</title>${lines}<path class="hm-area" d="${area}"/><path class="hm-line" d="${line}"/>${dates}</svg>`;
}
function hmDownsample(data,max=380){if(data.length<=max)return data;const step=Math.ceil(data.length/max);const result=[];for(let i=0;i<data.length;i+=step){const c=data.slice(i,i+step);result.push(c.reduce((a,b)=>b.v<a.v?b:a));result.push(c.reduce((a,b)=>b.v>a.v?b:a));}result.sort((a,b)=>a.t-b.t);return result;}
async function hmHistory(hass,id,days){
 if(!id)return [];
 const end=new Date(),start=new Date(end.getTime()-days*86400000);
 const path=`history/period/${encodeURIComponent(start.toISOString())}?filter_entity_id=${encodeURIComponent(id)}&end_time=${encodeURIComponent(end.toISOString())}&minimal_response&no_attributes`;
 const res=await hass.callApi('GET',path);
 const rows=Array.isArray(res)?(res.find(r=>Array.isArray(r)&&r.some(s=>s.entity_id===id))||res[0]||[]):[];
 const clean=rows.map(r=>({v:hmNumber(r.state),t:Date.parse(r.last_changed||r.last_updated)})).filter(p=>Number.isFinite(p.v)&&Number.isFinite(p.t)).sort((a,b)=>a.t-b.t);
 return hmDownsample(clean);
}
function hmConnectedPeriod(hass,map){const st=hmEntity(hass,map,'total');if(!st)return '';const x=st.last_updated||st.last_changed;return x?'Letzter Wert: '+new Date(x).toLocaleString('de-DE'):'';}
function hmOverview(hass,map,chartDays=7,chartHtml='',small=false){
 const tot=hmValue(hass,map,'total'),single=hmValue(hass,map,'single'),pct=hmValue(hass,map,'percent'),cnt=hmValue(hass,map,'tankCount'),range=hmValue(hass,map,'range'),low=hmBinary(hass,map,'low'),fault=hmBinary(hass,map,'fault');
 const ratio=Number.isFinite(pct)?Math.max(0,Math.min(100,pct)):0;
 const ann=low===true?'Niedrigstand – beide Tankanzeigen prüfen':fault===true?'Sensorstatus prüfen':Number.isFinite(tot)?'Hochrechnung aus einem gemessenen Tank':'Füllstandsdaten fehlen';
 return `<div class="hm-hero"><section class="hm-panel"><div class="hm-label">Geschätzter Ölbestand · Gesamt</div><div class="hm-total">${hmFmt(tot,1)} <span class="hm-unit">Liter</span></div><div class="hm-small">${hmEscape(ann)}</div><div class="hm-bar"><div style="width:${ratio}%"></div></div><div class="hm-split"><div><div class="hm-label">Gemessener Einzeltank</div><strong>${hmFmt(single,1)} L</strong></div><div style="text-align:right"><div class="hm-label">Tankanzahl</div><strong>${hmFmt(cnt,0)}</strong></div></div></section><section class="hm-panel"><h2>Füllstand</h2><div class="hm-meter"><div class="hm-ring" style="--pct:${ratio}%"><span>${hmFmt(pct,1)}%</span></div><div><div class="hm-label">Status</div><b>${fault===true?'Sensor prüfen':low===true?'Ölstand niedrig':'Überwachung aktiv'}</b><div class="hm-small" style="margin-top:7px">${hmEscape(hmConnectedPeriod(hass,map))}</div></div></div><div class="hm-note">Der zweite Tank wird nicht separat gemessen. Gesamtbestand und Reichweite sind Schätzungen.</div></section></div>
 <div class="hm-mini">${hmStateTile(hass,map,'daily','Heute','L')}${hmStateTile(hass,map,'weekly','Diese Woche','L')}${hmStateTile(hass,map,'monthly','Dieser Monat','L')}${hmStateTile(hass,map,'range','Restreichweite','Tage',0)}</div>
 ${small?'':`<div class="hm-section hm-panel"><div class="hm-charthead"><h2>Bestandsverlauf</h2><div class="hm-chartnav">${[1,7,30].map(n=>`<button data-days="${n}" class="${chartDays===n?'active':''}">${n===1?'24 h':n+' Tage'}</button>`).join('')}</div></div><div id="hm-history">${chartHtml||'<div class="hm-empty">Verlauf wird geladen …</div>'}</div><div class="hm-legend"><span><i></i>Geschätzte Gesamtmenge (L)</span></div></div>`}
 <div class="hm-footer">Nur informativ: Tankanzahl und Referenzhöhe beeinflussen die Hochrechnung. Niedrigstandwarnungen sollten immer mit den mechanischen Tankanzeigen abgeglichen werden.</div>`;
}
function hmAnalysis(hass,map){
 const values=[['Heute','daily','L'],['Diese Woche','weekly','L'],['Dieser Monat','monthly','L'],['Dieses Jahr','yearly','L'],['Letzte Befüllung','lastFill','L'],['Erkannte Befüllungen','fillCount',''],['Median über 6 h','median','L']];
 const enabled=hass?.states?.[map.verbrauchAktiv]?.state==='on';
 const status=hass?.states?.[map.verbrauchStatus]?.state;
 const reason=(status&&status!=='unknown'&&status!=='unavailable')?hmEscape(status):'Noch keine Diagnose verfügbar';
 return `<div class="hm-mini three" style="margin:0">${values.map(([label,key,unit])=>hmStateTile(hass,map,key,label,unit,key==='fillCount'?0:1)).join('')}</div>
 <div class="hm-section hm-panel"><h2>Verbrauchsanalyse</h2><p class="hm-small">Auswertung: <strong>${enabled?'aktiv':'deaktiviert oder noch nicht eingerichtet'}</strong>. Freigabe über den Home-Assistant-Helfer „Ölverbrauchsauswertung aktiviert“ nach kontrollierter Tankkalibrierung.</p>
 <p class="hm-small">Diagnose: ${reason}</p>
 <div class="hm-note">Die Schätzung wertet alle fünf Minuten den geglätteten Gesamtbestand aus. Änderungen unter 4 L werden als Hysterese zurückgehalten; Abnahmen über 20 L je Auswertung werden nicht automatisch als Verbrauch gebucht. Ein möglicher Tankanstieg ab 80 L braucht eine zweite passende Messung. Nach einer Unterbrechung oder Kalibrieränderung kann eine manuelle Neu-Referenzierung erforderlich sein. Keine Abrechnungs- oder Eichmessung.</div></div>`;
}
function hmSensors(hass,map){const fault=hmBinary(hass,map,'fault'),low=hmBinary(hass,map,'low');return `<div class="hm-mini three" style="margin:0">${hmStateTile(hass,map,'distance','Tankabstand · 5 Min','cm')}${hmStateTile(hass,map,'wifi','WLAN-Pegel','dBm',0)}${hmStateTile(hass,map,'tankCount','Anzahl Tanks','',0)}${hmTile('Sensor',''+(fault===true?'Störung':fault===false?'OK':'Unbekannt'))}${hmTile('Niedrigstand',low===true?'Warnung':low===false?'OK':'Unbekannt')}${hmStateTile(hass,map,'ref','Referenzbestand','L')}</div>`;}
function hmSettings(hass,map,config,mode){const states=hass?.states||{}, keys=Object.keys(states).filter(x=>/^(sensor|binary_sensor|input_number|input_boolean|switch)\./.test(x)).sort();const options=(selected,domain)=>`<option value="">Automatisch erkennen</option>${keys.filter(k=>k.startsWith(domain+'.')).map(k=>`<option value="${hmEscape(k)}" ${k===selected?'selected':''}>${hmEscape(states[k]?.attributes?.friendly_name||k)} — ${hmEscape(k)}</option>`).join('')}`;const fields=HM_FIELDS.filter(([key])=>mode==='card'?['total','single','percent','daily','weekly','monthly','range','fault','low'].includes(key):true);return `<section class="hm-panel"><h2>Entitäten zuordnen</h2><div class="hm-small" style="margin-bottom:13px">Automatische Erkennung funktioniert anhand der Entitätsnamen. Die Zuordnung gilt nur für diesen Browser. Standard: Slot 1 = Vorlauf, Slot 2 = Rücklauf. Die Messstellen kannst du hier ändern; ein eigener Spreizungs-Helfer ist optional. Globale Einstellungen lassen sich in der Panel-Konfiguration vorgeben.</div><div class="hm-settings">${fields.map(([key,label,domain])=>`<div class="hm-field"><label for="hm-field-${key}">${hmEscape(label)}</label><select id="hm-field-${key}" data-entity="${key}">${options(config[key]||'',domain)}</select></div>`).join('')}</div><div class="hm-toolbar"><button class="hm-btn" data-action="save-settings">Einstellungen speichern</button><button class="hm-btn secondary" data-action="reset-settings">Auto-Erkennung</button></div></section>`;}
function hmFooterVersion(){return `<div class="hm-footer">Keller Sensorzentrale v${HM_VERSION} · Keine Cloud, keine zusätzlichen Bibliotheken · Messwerte stammen aus Home Assistant</div>`;}



const KS_CYCLE_IDS=[
 'input_boolean.keller_heizkreis_auswertung_aktiv','binary_sensor.keller_heizkreis_aktiv',
 'sensor.keller_heizkreis_starts_heute','sensor.keller_heizkreis_starts_letzte_stunde',
 'sensor.keller_heizkreis_laufzeit_heute','sensor.keller_heizkreis_pause_heute',
 'sensor.keller_heizkreis_laufzeit_durchschnitt','sensor.keller_heizkreis_pause_durchschnitt',
 'sensor.keller_heizkreis_taktbewertung',
 'sensor.keller_heizkreis_vorlauf_mittel_24h','sensor.heizkreis_vorlauf_mittel_24h',
 'sensor.keller_heizkreis_ruecklauf_mittel_24h','sensor.heizkreis_ruecklauf_mittel_24h',
 'sensor.keller_heizkreis_spreizung_mittel_24h','sensor.heizkreis_spreizung_mittel_24h',
 'sensor.keller_heizkreis_vorlauf_max_24h','sensor.heizkreis_vorlauf_max_24h',
 'sensor.keller_heizkreis_ruecklauf_min_24h','sensor.heizkreis_ruecklauf_min_24h',
 'sensor.keller_heizkreis_spreizung_max_24h','sensor.heizkreis_spreizung_max_24h'
];
function ksCycleReading(states,id){
  // Home Assistant generates the entity_id of a statistics sensor from its
  // display name, not its unique_id. The six original YAML names lack "Keller".
  const direct=hmNumber(states[id]?.state);
  if(direct!==null)return direct;
  if(id.startsWith('sensor.keller_heizkreis_')){
   const fromName=id.replace('sensor.keller_heizkreis_','sensor.heizkreis_');
   const renamed=hmNumber(states[fromName]?.state);
   if(renamed!==null)return renamed;
  }
  return null;
}
function ksCyclePanel(hass){
 const states=hass?.states||{},flag=states['input_boolean.keller_heizkreis_auswertung_aktiv'];
 if(!flag)return '<div class="hm-section hm-panel"><h2>Heiztakt-Analyse</h2><div class="hm-note">Optionales Home-Assistant-Package keller_heiztakte.yaml noch nicht installiert. Bitte in den packages-Ordner legen und Home Assistant neu starten.</div></div>';
 const enabled=flag.state==='on',detected=states['binary_sensor.keller_heizkreis_aktiv']?.state;
 const phase=!enabled?'Auswertung deaktiviert':detected==='on'?'Temperatur-Aktivphase':detected==='off'?'Temperatur-Pause':'Noch keine zuverlässige Erkennung';
 const stats=[
  ['Erkannte Starts heute','sensor.keller_heizkreis_starts_heute','',0],
  ['Aktivphasen letzte 60 min','sensor.keller_heizkreis_starts_letzte_stunde','',0],
  ['Aktivphasen letzte 24 h','sensor.keller_heizkreis_starts_24h','',0],
  ['Erkannte Starts diese Woche','sensor.keller_heizkreis_starts_woche','',0],
  ['Aktivzeit heute','sensor.keller_heizkreis_laufzeit_heute','h',2],
  ['Pausenzeit heute','sensor.keller_heizkreis_pause_heute','h',2],
  ['Ø Aktivphase (seit Beginn)','sensor.keller_heizkreis_laufzeit_durchschnitt','min',1],
  ['Ø Pause (seit Beginn)','sensor.keller_heizkreis_pause_durchschnitt','min',1],
  ['Letzte Aktivphase','input_number.keller_heizkreis_laufzeit_letzte','min',1],
  ['Letzte Pause','input_number.keller_heizkreis_pause_letzte','min',1],
  ['Kürzeste Aktivphase','input_number.keller_heizkreis_laufzeit_min','min',1],
  ['Längste Aktivphase','input_number.keller_heizkreis_laufzeit_max','min',1],
  ['Kürzeste Pause','input_number.keller_heizkreis_pause_min','min',1],
  ['Längste Pause','input_number.keller_heizkreis_pause_max','min',1],
  ['Ø Vorlauf · 24 h','sensor.keller_heizkreis_vorlauf_mittel_24h','°C',1],
  ['Ø Rücklauf · 24 h','sensor.keller_heizkreis_ruecklauf_mittel_24h','°C',1],
  ['Ø Spreizung · 24 h','sensor.keller_heizkreis_spreizung_mittel_24h','K',1],
  ['Vorlauf Maximum · 24 h','sensor.keller_heizkreis_vorlauf_max_24h','°C',1],
  ['Rücklauf Minimum · 24 h','sensor.keller_heizkreis_ruecklauf_min_24h','°C',1],
  ['Spreizung Maximum · 24 h','sensor.keller_heizkreis_spreizung_max_24h','K',1]
 ];
 const list=stats.map(([title,id,u,places])=>hmTile(title,hmFmt(ksCycleReading(states,id),places),u)).join('');
 const rating=states['sensor.keller_heizkreis_taktbewertung']?.state||'–';
 return '<div class="hm-section hm-panel"><div class="hm-charthead"><h2>Heiztakt-Analyse</h2><button class="hm-btn secondary" data-cycle-action="'+(enabled?'off':'on')+'">'+(enabled?'Auswertung pausieren':'Auswertung aktivieren')+'</button></div>'+
  '<p class="hm-small"><b>'+hmEscape(phase)+'</b> · Einschätzung: '+hmEscape(enabled?rating:'nicht aktiv')+'</p>'+
  '<div class="hm-mini three">'+list+'</div>'+
  '<div class="hm-note" style="margin-top:14px">Abgeleitete Temperaturzyklen, keine exakten Brennerstarts oder Brennerlaufzeiten. Hysterese und Zeitfilter vermeiden viele Messschwankungen. Die Werte entstehen erst nach Aktivierung; Langzeitstatistiken benötigen den HA-Recorder. Durchschnitt/Extremwerte gelten für vollständig erkannte Phasen seit Start der Erfassung.</div></div>';
}
async function ksCycleIntervals(hass,days){
 const id='binary_sensor.keller_heizkreis_aktiv';if(!hass?.states?.[id])return [];
 const end=Date.now(),start=end-days*86400000;
 const url='history/period/'+encodeURIComponent(new Date(start).toISOString())+'?filter_entity_id='+encodeURIComponent(id)+'&end_time='+encodeURIComponent(new Date(end).toISOString())+'&minimal_response&no_attributes';
 const data=await hass.callApi('GET',url);
 const rows=Array.isArray(data)?(data.find(r=>Array.isArray(r)&&r.some(p=>p.entity_id===id))||data[0]||[]):[];
 const changes=rows.map(r=>({s:r.state,t:Date.parse(r.last_changed||r.last_updated)})).filter(r=>Number.isFinite(r.t)).sort((a,b)=>a.t-b.t);
 let opened=null;const out=[];
 for(const r of changes){if(r.s==='on'&&opened===null)opened=Math.max(start,r.t);else if(r.s!=='on'&&opened!==null){if(r.t>opened)out.push([opened,r.t]);opened=null;}}
 if(opened!==null)out.push([opened,end]);
 return out;
}

const KS_TEMP_COLORS=['#35b1e2','#faad4c','#52c29a','#c5a1ef','#e7768e','#c6dc65'];
function ksSlotName(hass,id,num){
 const ent=hass?.states?.[id];
 const friendly=ent?.attributes?.friendly_name||'';
 // HA rename is displayed; generic MQTT name is shortened.
 const trim=friendly.replace(/^.*?\b(Slot\s*[1-6])$/i,'$1');
 return (trim && trim!==friendly)?trim:(friendly||`Slot ${num}`);
}
function ksTemps(hass,map){
 return `<div class="hm-sensor-grid">${Array.from({length:6},(_,i)=>{
 const id=map['temp'+(i+1)];const val=hmValue(hass,map,'temp'+(i+1));
 const label=id?ksSlotName(hass,id,i+1):`Slot ${i+1}`;
 return `<div class="hm-sensor"><span class="hm-label" title="${hmEscape(id||'Nicht zugeordnet')}">${hmEscape(label)}</span><strong>${hmFmt(val,1)} ${Number.isFinite(val)?'°C':''}</strong></div>`;
 }).join('')}</div>`;
}
function ksRelays(hass,map){
 return `<div class="hm-relays">${[1,2].map(i=>{
 const id=map['relay'+i],ent=hass?.states?.[id];const avail=ent&&['on','off'].includes(ent.state);const on=ent?.state==='on';
 return `<section class="hm-relay"><div class="hm-label">${hmEscape(ent?.attributes?.friendly_name||'Relais '+i)}</div><div class="hm-row"><b>${!avail?'Nicht verfügbar':on?'EIN':'AUS'}</b><span class="hm-small">${avail?'MQTT-Schalter':'Zuordnung prüfen'}</span></div><div class="hm-toolbar"><button class="hm-btn ${on?'is-on':''}" data-relay="${i}" data-state="on" ${!avail||on?'disabled':''}>EIN</button><button class="hm-btn secondary ${!on&&avail?'is-off':''}" data-relay="${i}" data-state="off" ${!avail||!on?'disabled':''}>AUS</button></div></section>`;
 }).join('')}</div><p class="hm-small" style="margin-top:8px">Relais nur für ungefährliche Kleinspannungs-Testlasten verwenden; keine sicherheitsrelevante Heizungssteuerung.</p>`;
}
async function ksRelayClick(component,button){
 const relay=Number(button.dataset.relay);if(relay!==1&&relay!==2)return;
 const desired=button.dataset.state;
 if(desired!=='on'&&desired!=='off')return;
 if(component._relayBusy||Date.now()-(component._relayLast||0)<950)return;
 const id=component._map()['relay'+relay];
 const state=component._hass?.states?.[id];
 if(!hmSafeId(id)||!id.startsWith('switch.')||!state||!['on','off'].includes(state.state))return;
 component._relayBusy=true;component._relayLast=Date.now();
 button.disabled=true;
 try{await component._hass.callService('switch',desired==='on'?'turn_on':'turn_off',{entity_id:id});}
 catch(err){component._relayError='Relais '+relay+': Schalten fehlgeschlagen. '+String(err?.message||'Berechtigung/Verbindung prüfen').slice(0,130);}
 finally{component._relayBusy=false;component._schedule(true);}
}

function ksHeatTiles(hass,map){
 const a=hmValue(hass,map,'flow'),b=hmValue(hass,map,'return');
 const difference=map.flow!==map.return&&Number.isFinite(a)&&Number.isFinite(b)?a-b:null;
 return '<div class="hm-mini three" style="margin-top:0">'+
   hmTile('Heizung Vorlauf',hmFmt(a,1),'°C')+hmTile('Heizung Rücklauf',hmFmt(b,1),'°C')+
   hmTile('Spreizung (Vorlauf − Rücklauf)',hmFmt(difference,1),'K')+
   '</div><p class="hm-small" style="margin-top:10px">Standard: Slot 1 = Vorlauf, Slot 2 = Rücklauf. Unter Einstellungen frei änderbar.</p>';
}
function ksHeatSection(hass,map,days,graph){
 return '<div class="hm-section hm-panel"><div class="hm-charthead"><h2>Heizkreis · Vorlauf, Rücklauf & Spreizung</h2>'+ksTimeNav(days)+'</div>'+
    ksHeatTiles(hass,map)+'<div id="ks-heating-history" style="margin-top:15px">'+
    (graph||'<div class="hm-empty">Heizkreisverlauf wird geladen …</div>')+'</div></div>';
}
function ksDeltaHistory(flow,ret){
 // Asynchrone Recorder-Änderungen zeitlich zusammenführen; keine Paarung
 // von Quellen, deren letzte bestätigte Werte mehr als 2 Stunden auseinanderliegen.
 const points=[...flow.map(p=>({...p,side:0})),...ret.map(p=>({...p,side:1}))]
   .filter(p=>Number.isFinite(p.v)&&Number.isFinite(p.t)).sort((a,b)=>a.t-b.t);
 const seen=[null,null],out=[];
 for(const p of points){
  seen[p.side]=p;
  if(!seen[0]||!seen[1]||Math.abs(seen[0].t-seen[1].t)>7200000)continue;
  const v=seen[0].v-seen[1].v;
  if(out.length&&out[out.length-1].t===p.t)out[out.length-1]={t:p.t,v};
  else out.push({t:p.t,v});
 }
 return hmDownsample(out);
}
async function ksHeatHistory(hass,map,days){
 if(!hmSafeId(map.flow)||!hmSafeId(map.return)||map.flow===map.return||
   !hass?.states?.[map.flow]||!hass?.states?.[map.return])
  return '<div class="hm-empty">Bitte Vorlauf und Rücklauf in Einstellungen auf zwei unterschiedliche Temperatur-Entitäten legen.</div>';
 const delta=map.spread&&hass.states[map.spread]&&map.spread!==map.flow&&map.spread!==map.return?map.spread:null;
 const [flow,ret,saved,intervals]=await Promise.all([
  hmHistory(hass,map.flow,days),hmHistory(hass,map.return,days),
  delta?hmHistory(hass,delta,days).catch(()=>[]):Promise.resolve([]),
  ksCycleIntervals(hass,days).catch(()=>[])
 ]);
 return ksMultiSvg([
  {label:'Vorlauf',color:'#faad4c',points:flow},
  {label:'Rücklauf',color:'#35b1e2',points:ret},
  {label:'Spreizung',color:'#c5a1ef',axis:'spread',points:saved.length>=2?saved:ksDeltaHistory(flow,ret)}
 ],intervals);
}

function ksTimeNav(days){return `<div class="hm-chartnav">${[1,7,30].map(n=>`<button data-temp-days="${n}" class="${days===n?'active':''}">${n===1?'24 h':n+' Tage'}</button>`).join('')}</div>`;}
function ksMultiSvg(series,intervals=[]){
 const active=series.filter(s=>s.points?.length>=2);
 if(!active.length)return '<div class="hm-empty">Noch kein Temperaturverlauf. Sensoren zuordnen und Recorder aktivieren.</div>';
 const pts=active.flatMap(s=>s.points),start=Math.min(...pts.map(p=>p.t)),end=Math.max(...pts.map(p=>p.t));
 const tempPts=active.filter(s=>s.axis!=='spread').flatMap(s=>s.points.map(p=>p.v));
 const diffPts=active.filter(s=>s.axis==='spread').flatMap(s=>s.points.map(p=>p.v));
 const bounds=arr=>{if(!arr.length)return [0,1];let lo=Math.min(...arr),hi=Math.max(...arr);const pad=Math.max(.6,(hi-lo)*.12);return [lo-pad,hi+pad];};
 const c=bounds(tempPts),k=bounds(diffPts);
 const w=980,h=265,l=57,r=diffPts.length?70:18,t=18,b=31;
 const px=x=>l+(x-start)/Math.max(1,end-start)*(w-l-r);
 const py=(v,axis)=>{const [lo,hi]=axis==='spread'?k:c;return t+(hi-v)/(hi-lo)*(h-t-b);};
 const lines=Array.from({length:4},(_,i)=>{const f=i/3,y=t+f*(h-t-b);
   return '<line class="hm-gridline" x1="'+l+'" x2="'+(w-r)+'" y1="'+y+'" y2="'+y+'"/>'+
    (tempPts.length?'<text x="'+(l-8)+'" y="'+(y+4)+'" text-anchor="end">'+hmFmt(c[1]-f*(c[1]-c[0]),1)+'°</text>':'')+
    (diffPts.length?'<text x="'+(w-r+9)+'" y="'+(y+4)+'">'+hmFmt(k[1]-f*(k[1]-k[0]),1)+' K</text>':'');
 }).join('');
 const dates=[0,.5,1].map(a=>{const dt=start+(end-start)*a;return '<text x="'+px(dt).toFixed(1)+'" y="'+(h-8)+'" text-anchor="middle">'+hmDate(dt,end-start<=86400000)+'</text>';}).join('');
 const gap=Math.max(3*3600000,(end-start)*.035);
 const bands=intervals.filter(([a,b])=>b>start&&a<end).map(([a,b])=>{const lft=px(Math.max(a,start)),right=px(Math.min(b,end));return '<rect x="'+lft.toFixed(1)+'" y="'+t+'" width="'+Math.max(0,right-lft).toFixed(1)+'" height="'+(h-t-b)+'" fill="rgba(241,162,67,.10)"/>';}).join('');
 const paths=active.map(s=>{let last=-Infinity;
  const coords=s.points.filter(p=>Number.isFinite(p.v)&&Number.isFinite(p.t)).map(p=>{
   const cmd=p.t-last>gap?'M':'L';last=p.t;return cmd+px(p.t).toFixed(1)+' '+py(p.v,s.axis).toFixed(1);
  }).join(' ');
  return '<path d="'+coords+'" fill="none" stroke="'+s.color+'" stroke-width="2.5"'+(s.axis==='spread'?' stroke-dasharray="8 5"':'')+' stroke-linejoin="round" stroke-linecap="round"/>';
 }).join('');
 return '<svg class="hm-graph" viewBox="0 0 '+w+' '+h+'" role="img" aria-label="Temperaturverlauf, bei Heizkreis: °C links und K rechts"><title>Temperaturverlauf</title>'+lines+bands+paths+dates+'</svg>'+
  '<div class="hm-graphkeys">'+active.map(s=>'<span><i style="background:'+s.color+'"></i>'+hmEscape(s.label)+(s.axis==='spread'?' (K)':' (°C)')+'</span>').join('')+'</div>'+
  (diffPts.length?'<p class="hm-small">Linke Skala: °C · rechte Skala: Kelvin (K). Gestrichelt: Spreizung.</p>':'');
}
async function ksTemperatureHistory(hass,map,days){
 const tasks=Array.from({length:6},(_,i)=>({i,id:map['temp'+(i+1)]})).filter(t=>t.id&&hass?.states?.[t.id]);
 const result=await Promise.all(tasks.map(async t=>{
  try{return {label:ksSlotName(hass,t.id,t.i+1),color:KS_TEMP_COLORS[t.i],points:await hmHistory(hass,t.id,days)}}
  catch(e){return {label:'Slot '+(t.i+1),color:KS_TEMP_COLORS[t.i],points:[]}}
 }));
 return ksMultiSvg(result);
}
function ksOverview(hass,map){return `<div class="hm-section hm-panel"><h2>Relaissteuerung</h2>${ksRelays(hass,map)}</div><div class="hm-section hm-panel"><h2>Temperatur-Slots · 1–6</h2>${ksTemps(hass,map)}</div><div class="hm-section hm-panel"><h2>Auswertungsbereiche</h2><p class="hm-small">Heizkreis und Taktung sind unter <b>Heizung & Takte</b>. Ölbestand und Ölverbrauch sind getrennt in <b>Tank & Verlauf</b> und <b>Ölverbrauch</b>.</p><div class="hm-toolbar"><button class="hm-btn" data-tab="heating">Heizung & Takte</button><button class="hm-btn secondary" data-tab="tank">Tank & Verlauf</button></div></div>`;}

class HeizoelMonitorPanel extends HTMLElement {
 constructor(){super();this.attachShadow({mode:'open'});this._tab='overview';this._range=7;this._tempDays=1;this._historyCache=new Map();this._tempCache=new Map();this._heatCache=new Map();this._historyText='';this._tempText='';this._heatText='';this._lastSig='';this._lastRenderAt=0;this._pending=false;this._local={};try{this._local=JSON.parse(localStorage.getItem(HM_SAVE_KEY)||'{}')||{}}catch(_){}}
 set hass(hass){this._hass=hass;this._schedule();}
 set panel(panel){this._panel=panel;this._schedule(true);}
 set narrow(v){this._narrow=v;}
 connectedCallback(){this.shadowRoot.addEventListener('click',(ev)=>this._click(ev));this._schedule(true);}
 _base(){return {...(this._panel?.config?.entities||{}),...this._local};}
 _map(){return hmAutoMap(this._hass,this._base());}
 _sig(){const m=this._map(),h=this._hass;return HM_FIELDS.map(([k])=>m[k]+':'+(h?.states?.[m[k]]?.state||'')).join('|')+'|'+KS_CYCLE_IDS.map(k=>h?.states?.[k]?.state||'').join('|')+'|'+this._tab+'|'+this._range+'|'+this._tempDays;}
 _schedule(force=false){if(!this.isConnected||!this._hass)return;if(this._pending)return;const sig=this._sig();if(!force&&sig===this._lastSig)return;if(!force&&this._tab==='settings')return;if(!force&&Date.now()-this._lastRenderAt<1500)return;this._pending=true;Promise.resolve().then(()=>{this._pending=false;if(this.isConnected)this._render()});}
 _click(ev){const item=ev.target.closest('button');if(!item)return;
  if(item.dataset.relay){ksRelayClick(this,item);return;}
   if(item.dataset.cycleAction){if(this._hass?.states?.['input_boolean.keller_heizkreis_auswertung_aktiv'])this._hass.callService('input_boolean',item.dataset.cycleAction==='on'?'turn_on':'turn_off',{entity_id:'input_boolean.keller_heizkreis_auswertung_aktiv'}).catch(()=>{});return;}
  if(item.dataset.action==='toggle-menu'){this.dispatchEvent(new Event('hass-toggle-menu',{bubbles:true,composed:true}));return;}
  if(item.dataset.tab){this._tab=item.dataset.tab;this._lastSig='';this._render();return;}
  if(item.dataset.days){this._range=Number(item.dataset.days);this._historyText='';this._lastSig='';this._render();return;}
  if(item.dataset.tempDays){this._tempDays=Number(item.dataset.tempDays);this._tempText='';this._heatText='';this._lastSig='';this._render();return;}
  if(item.dataset.action==='save-settings'){const next={};for(const select of this.shadowRoot.querySelectorAll('[data-entity]'))if(hmSafeId(select.value))next[select.dataset.entity]=select.value;this._local=next;try{localStorage.setItem(HM_SAVE_KEY,JSON.stringify(next))}catch(_){}this._historyCache.clear();this._tempCache.clear();this._heatCache.clear();this._heatText='';this._lastSig='';this._tab='overview';this._render();return;}
  if(item.dataset.action==='reset-settings'){this._local={};try{localStorage.removeItem(HM_SAVE_KEY)}catch(_){}this._historyCache.clear();this._tempCache.clear();this._heatCache.clear();this._heatText='';this._lastSig='';this._render();return;}
  if(item.dataset.action==='reload-history'){this._historyCache.clear();this._tempCache.clear();this._fetchOil(true);this._fetchTemp(true);this._fetchHeat(true);return;}
 }
 _render(){if(!this._hass||!this.isConnected)return;const map=this._map(),s=hmStatus(this._hass,map),t=this._tab;
  const sections={overview:'Übersicht',heating:'Heizung & Takte',temperatures:'Temperatur-Slots',tank:'Tank & Verlauf',analysis:'Ölverbrauch',settings:'Einstellungen'};
  let body=t==='overview'?ksOverview(this._hass,map):
    t==='heating'?`${ksHeatSection(this._hass,map,this._tempDays,this._heatText)}${ksCyclePanel(this._hass)}`:
   t==='temperatures'?`<div class="hm-panel"><h2>Temperaturen · Slot 1–6</h2>${ksTemps(this._hass,map)}</div><div class="hm-section hm-panel"><div class="hm-charthead"><h2>Temperaturverlauf</h2>${ksTimeNav(this._tempDays)}</div><div id="ks-temperature-history">${this._tempText||'<div class="hm-empty">Lade Verlauf …</div>'}</div></div>`:
   t==='tank'?`<div class="hm-panel">${hmOverview(this._hass,map,this._range,this._historyText)}</div>`:
   t==='analysis'?hmAnalysis(this._hass,map):
   hmSettings(this._hass,map,this._base(),'panel');
  this.shadowRoot.innerHTML=`<style>${HM_STYLES}</style><main class="hm-wrap"><header class="hm-head"><div style="display:flex;align-items:center;gap:12px"><button class="hm-menu" data-action="toggle-menu" aria-label="Home-Assistant-Menü öffnen">☰</button><div><h1>Kellersteuerung</h1><p>Sensoren · Tank · Relais · Verläufe</p></div></div><span class="hm-status ${s.ok?'':'bad'}"><span class="hm-dot"></span>${hmEscape(s.text)}</span></header><nav class="hm-tabs" aria-label="Kellersteuerung Bereiche">${Object.entries(sections).map(([id,name])=>`<button data-tab="${id}" class="${t===id?'active':''}">${name}</button>`).join('')}</nav>${body}${hmFooterVersion()}</main>`;
  this._lastSig=this._sig();this._lastRenderAt=Date.now();if(t==='tank')this._fetchOil();if(t==='temperatures')this._fetchTemp();if(t==='heating')this._fetchHeat();
 }
 async _fetchOil(force=false){if(!this._hass)return;const id=this._map().total,key=this._range+':'+id,entry=this._historyCache.get(key),node=this.shadowRoot.getElementById('hm-history');if(!node)return;
  if(!force&&entry&&Date.now()-entry.time<300000){this._historyText=entry.html;node.innerHTML=entry.html;return;}if(this._oilLoading===key)return;this._oilLoading=key;
  try{const html=hmSvg(await hmHistory(this._hass,id,this._range));this._historyCache.set(key,{html,time:Date.now()});if(this._oilLoading===key){this._historyText=html;const n=this.shadowRoot.getElementById('hm-history');if(n)n.innerHTML=html;}}
  catch(_){const n=this.shadowRoot.getElementById('hm-history');if(n)n.innerHTML='<div class="hm-empty hm-error">Tankverlauf momentan nicht verfügbar (Recorder prüfen)</div>';}
  finally{if(this._oilLoading===key)this._oilLoading='';}
 }
 async _fetchTemp(force=false){if(!this._hass)return;const m=this._map(),key=this._tempDays+':'+Array.from({length:6},(_,i)=>m['temp'+(i+1)]||'').join(':'),entry=this._tempCache.get(key),node=this.shadowRoot.getElementById('ks-temperature-history');if(!node)return;
  if(!force&&entry&&Date.now()-entry.time<300000){this._tempText=entry.html;node.innerHTML=entry.html;return;}if(this._tempLoading===key)return;this._tempLoading=key;
  try{const html=await ksTemperatureHistory(this._hass,m,this._tempDays);this._tempCache.set(key,{html,time:Date.now()});if(this._tempLoading===key){this._tempText=html;const n=this.shadowRoot.getElementById('ks-temperature-history');if(n)n.innerHTML=html;}}
  catch(_){const n=this.shadowRoot.getElementById('ks-temperature-history');if(n)n.innerHTML='<div class="hm-empty hm-error">Temperaturverlauf momentan nicht verfügbar (Recorder prüfen)</div>';}
  finally{if(this._tempLoading===key)this._tempLoading='';}
 }
 async _fetchHeat(force=false){
  if(!this._hass)return;
  const m=this._map(),key=this._tempDays+':'+[m.flow||'',m.return||'',m.spread||''].join(':');
  const node=this.shadowRoot.getElementById('ks-heating-history'),entry=this._heatCache.get(key);
  if(!node)return;
  if(!force&&entry&&Date.now()-entry.time<60000){this._heatText=entry.html;node.innerHTML=entry.html;return;}
  if(this._heatLoading===key)return;
  this._heatLoading=key;
  try{
   const html=await ksHeatHistory(this._hass,m,this._tempDays);
   this._heatCache.set(key,{html,time:Date.now()});
   if(this._heatLoading===key){this._heatText=html;const n=this.shadowRoot.getElementById('ks-heating-history');if(n)n.innerHTML=html;}
  }catch(_){
   const n=this.shadowRoot.getElementById('ks-heating-history');if(n)n.innerHTML='<div class="hm-empty hm-error">Heizkreisverlauf nicht verfügbar (Recorder/Zuordnung prüfen).</div>';
  }finally{if(this._heatLoading===key)this._heatLoading='';}
 }

}
if(!customElements.get('heizoel-monitor-panel'))customElements.define('heizoel-monitor-panel',HeizoelMonitorPanel);
