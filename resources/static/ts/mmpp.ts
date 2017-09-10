/// <reference path="jquery.d.ts"/>
/// <reference path="mustache.d.ts"/>

import { jsonAjax, get_serial, spectrum_to_rgb, push_and_get_index } from "./utils";
import { Editor } from "./editor";
import { create_workset, load_workset, Workset, Renderer, RenderingStyles } from "./workset";

let workset : Workset;
let renderer : Renderer;

// Whether to include non essential steps or not
let include_non_essentials : boolean = false;

$(function() {
  jsonAjax("/api/version").done(function(version_data) {
    $("#version").text(`Application is ${version_data.application}. Versions between ${version_data.min_version} and ${version_data.max_version} are supported.`);
    $("#version").css("display", "block");
    $("#create_workset").css("display", "block");
  })
});

export function ui_create_workset() {
  create_workset(function (new_workset : Workset) {
    workset = new_workset;
    renderer = new Renderer(RenderingStyles.ALT_HTML, workset);
    workset_loaded();
  });
}

export function ui_load_workset_0() {
  load_workset(0, function (new_workset : Workset) {
    workset = new_workset;
    renderer = new Renderer(RenderingStyles.ALT_HTML, workset);
    workset_loaded();
  });
}

function workset_loaded() {
  $("#workset_status").text(`Workset ${workset.id} loaded!`);
  $("#create_workset").css("display", "none");
  $("#workset").css("display", "block");
}

export function ui_load_data() {
  workset.load_data(function() {
    $("#workset_status").text("Library data loaded!");
  });
}

export function show_statement() {
  if (workset.status !== "loaded") {
    return;
  }
  let label : string = $("#statement_label").val();
  let label_tok : number = workset.labels_inv[label];
  jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${label_tok}`).done(function(data) {;
    $("#editor").val(new Renderer(RenderingStyles.TEXT, workset).render_from_codes(data["sentence"]));
    $("#current_statement").html(new Renderer(RenderingStyles.HTML, workset).render_from_codes(data["sentence"]));
    $("#current_statement_alt").html(new Renderer(RenderingStyles.ALT_HTML, workset).render_from_codes(data["sentence"]));
  });
  $("#show_statement_div").css('display', 'block');
}

function render_proof_internal(proof_tree, depth : number, step : number) : [string, number] {
  let children : [string, number][] = proof_tree.children.filter(function(el) {
    return include_non_essentials ? true : el["essential"];
  }).map(function (el) {
    let proof : string;
    [proof, step] = render_proof_internal(el, depth+1, step);
    return [proof, step];
  });
  step += 1;
  return [Mustache.render($('#proof_templ').html(), {
    label: workset.labels[proof_tree.label],
    number: proof_tree.number > 0 ? proof_tree.number.toString() : "",
    number_color: spectrum_to_rgb(proof_tree.number, workset.max_number),
    sentence: renderer.render_from_codes(proof_tree.sentence),
    children: children.map(function(el) { return el[0]; }),
    children_steps: children.map(function(el) { return el[1]; }),
    dists: proof_tree["dists"].map(function(el) { return renderer.render_from_codes([el[0]]) + ", " + renderer.render_from_codes([el[1]]); }),
    indentation: ". ".repeat(depth) + (depth+1).toString(),
    essential: proof_tree["essential"],
    step: step,
  }), step];
}

function render_proof(proof_tree) {
  return render_proof_internal(proof_tree, 0, 0)[0];
}

export function show_assertion() {
  if (workset.status !== "loaded") {
    return;
  }

  // Resolve the label and request the corresponding assertion
  let label : string = $("#statement_label").val();
  let label_tok : number = workset.labels_inv[label];
  jsonAjax(`/api/1/workset/${workset.id}/get_assertion/${label_tok}`).done(function(data) {
    let assertion = data["assertion"];

    // Request all the interesting things for the proof
    let requests : JQueryXHR[] = [];
    let requests_map = {
      thesis: push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${assertion["thesis"]}`)),
      proof_tree: push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_proof_tree/${assertion["thesis"]}`)),
      ess_hyps_sent: assertion["ess_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${el}`)); }),
      float_hyps_sent: assertion["float_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${el}`)); }),
      /*ess_hyps_ass: assertion["ess_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_assertion/${el}`)); }),
      float_hyps_ass: assertion["float_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_assertion/${el}`)); }),*/
    };

    // Fire all the requests and then feed the results to the template
    $.when.apply($, requests).done(function() {
      let responses = arguments;
      $("#show_assertion_div").html(Mustache.render($('#assertion_templ').html(), {
        thesis: renderer.render_from_codes(responses[requests_map["thesis"]]["sentence"]),
        ess_hyps_sent: requests_map["ess_hyps_sent"].map(function(el) { return renderer.render_from_codes(responses[el]["sentence"]); }),
        float_hyps_sent: requests_map["float_hyps_sent"].map(function(el) { return renderer.render_from_codes(responses[el]["sentence"]); }),
        dists: assertion["dists"].map(function(el) { return renderer.render_from_codes([el[0]]) + ", " + renderer.render_from_codes([el[1]]); }),
        proof: render_proof(responses[requests_map["proof_tree"]]["proof_tree"]),
      }));
      $("#show_assertion_div").css('display', 'block');
    });
  });
}

function modifier_render_proof(proof_tree, parent_div : string) {
  if (!proof_tree["essential"] && !include_non_essentials) {
    return;
  }
  let id : string = get_serial();
  $("#" + parent_div).append(Mustache.render($("#modifier_step_templ").html(), {
    id: id,
    sentence: renderer.render_from_codes(proof_tree.sentence),
    label: workset.labels[proof_tree.label],
    number: proof_tree.number > 0 ? proof_tree.number.toString() : "",
    number_color: spectrum_to_rgb(proof_tree.number, workset.max_number),
  }));
  for (let child of proof_tree.children) {
    modifier_render_proof(child, `step_${id}_children`);
  }
}

export function show_modifier() {
  if (workset.status !== "loaded") {
    return;
  }

  // Resolve the label and request the corresponding assertion
  let label : string = $("#statement_label").val();
  let label_tok : number = workset.labels_inv[label];
  jsonAjax(`/api/1/workset/${workset.id}/get_assertion/${label_tok}`).done(function(data) {
    let assertion = data["assertion"];

    // Request all the interesting things for the proof
    let requests : JQueryXHR[] = [];
    let requests_map = {
      thesis: push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${assertion["thesis"]}`)),
      proof_tree: push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_proof_tree/${assertion["thesis"]}`)),
      ess_hyps_sent: assertion["ess_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${el}`)); }),
      float_hyps_sent: assertion["float_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${el}`)); }),
      /*ess_hyps_ass: assertion["ess_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_assertion/${el}`)); }),
      float_hyps_ass: assertion["float_hyps"].map(function (el) { return push_and_get_index(requests, jsonAjax(`/api/1/workset/${workset.id}/get_assertion/${el}`)); }),*/
    };

    // Fire all the requests and then feed the results to the template
    $.when.apply($, requests).done(function() {
      let responses = arguments;
      $("#show_assertion_div").html(Mustache.render($('#modifier_templ').html(), {}));
      modifier_render_proof(responses[requests_map["proof_tree"]]["proof_tree"], "modifier");
      $("#show_assertion_div").css('display', 'block');
    });
  });
}

export function editor_changed() {
  if (workset.status !== "loaded") {
    return;
  }
  let tokens : string[] = $("#editor").val().split(" ");
  $("#current_statement").html(new Renderer(RenderingStyles.HTML, workset).render_from_strings(tokens));
  $("#current_statement_alt").html(new Renderer(RenderingStyles.ALT_HTML, workset).render_from_strings(tokens));
}

/*function retrieve_sentence(label_tok : number) : number[] {
  let sentence : number[];
  jsonAjax(`/api/1/workset/${workset.id}/get_sentence/${label_tok}`, true, true, false).done(function(data) {
    sentence = data["sentence"];
  });
  return sentence;
}*/
