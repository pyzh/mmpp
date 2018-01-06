/// <reference path="jquery.d.ts"/>
/// <reference path="mustache.d.ts"/>

import { assert } from "./utils";
import { TreeManager, TreeNode } from "./tree";

export interface NodePainter {
  paint_node(node : TreeNode) : void;
  set_editor_manager(editor_manager : EditorManager) : void;
}

export class EditorManagerObject {
  visible : boolean = false;
  children_open : boolean;
  data2_open : boolean;
}

export class EditorManager extends TreeManager {
  parent_div : string;
  painter : NodePainter;

  constructor(parent_div : string, painter : NodePainter) {
    super();
    this.parent_div = parent_div;
    this.painter = painter;
    this.painter.set_editor_manager(this);
  }

  creating_node(node : TreeNode) : Promise< void > {
    let obj = new EditorManagerObject();
    this.set_manager_object(node, obj);
    if (node.is_root()) {
      obj.visible = true;
      let html_code = Mustache.render(STEP_ROOT_TMPL, {
        eid: node.get_tree().get_id(),
        id: node.get_id(),
      });
      $(`#${this.parent_div}`).html(html_code);
    }
    return Promise.resolve();
  }

  destroying_node(node : TreeNode) : Promise< void > {
    let obj : EditorManagerObject = this.get_manager_object(node);
    if (node.is_root()) {
      assert(obj.visible);
      obj.visible = false;
      $(`#${this.parent_div}`).html("");
    }
    return Promise.resolve();
  }

  after_reparenting(parent : TreeNode, child : TreeNode, idx : number) : void {
    let parent_obj : EditorManagerObject = this.get_manager_object(parent);
    let child_obj : EditorManagerObject = this.get_manager_object(child);
    assert(!child_obj.visible);
    if (parent_obj.visible) {
      this.make_subtree_visible(parent, child, idx);
    }
  }

  before_orphaning(parent : TreeNode, child : TreeNode, idx : number) : void {
    let parent_obj : EditorManagerObject = this.get_manager_object(parent);
    let child_obj : EditorManagerObject = this.get_manager_object(child);
    assert(parent_obj.visible === child_obj.visible);
    if (child_obj.visible) {
      this.make_subtree_hidden(child);
    }
    let full_id = this.compute_full_id(child);
    $(`#${full_id}`).remove();
  }

  compute_full_id(node : TreeNode) : string {
    return `${node.get_tree().get_id()}_step_${node.get_id()}`;
  }

  compute_data1_element(node : TreeNode) : JQuery {
    return $(`#${this.compute_full_id(node)}_data1`);
  }

  compute_data2_element(node : TreeNode) : JQuery {
    return $(`#${this.compute_full_id(node)}_data2`);
  }

  toggle_children(node : TreeNode, animation : boolean = true) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    let full_id = this.compute_full_id(node);
    if (obj.children_open) {
      $(`#${full_id}_btn_toggle_children`).removeClass("mini_button_open").addClass("mini_button_closed");
      if (animation) {
        $(`#${full_id}_children`).slideUp();
      } else {
        $(`#${full_id}_children`).hide();
      }
      obj.children_open = false;
    } else {
      $(`#${full_id}_btn_toggle_children`).removeClass("mini_button_closed").addClass("mini_button_open");
      if (animation) {
        $(`#${full_id}_children`).slideDown();
      } else {
        $(`#${full_id}_children`).show();
      }
      obj.children_open = true;
    }
  }

  open_children(node : TreeNode, animation : boolean = true) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    if (!obj.children_open) {
      this.toggle_children(node, animation);
    }
  }

  close_children(node : TreeNode, animation : boolean = true) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    if (obj.children_open) {
      this.toggle_children(node, animation);
    }
  }

  toggle_data2(node : TreeNode, animation : boolean = true) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    let full_id = this.compute_full_id(node);
    if (obj.data2_open) {
      $(`#${full_id}_btn_toggle_data2`).removeClass("mini_button_open").addClass("mini_button_closed");
      if (animation) {
        $(`#${full_id}_data2`).slideUp();
      } else {
        $(`#${full_id}_data2`).hide();
      }
      obj.data2_open = false;
    } else {
      $(`#${full_id}_btn_toggle_data2`).removeClass("mini_button_closed").addClass("mini_button_open");
      if (animation) {
        $(`#${full_id}_data2`).slideDown();
      } else {
        $(`#${full_id}_data2`).show();
      }
      obj.data2_open = true;
    }
  }

  open_data2(node : TreeNode, animation : boolean = true) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    if (!obj.data2_open) {
      this.toggle_data2(node, animation);
    }
  }

  close_data2(node : TreeNode, animation : boolean = true) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    if (obj.data2_open) {
      this.toggle_data2(node, animation);
    }
  }

  make_subtree_visible(parent : TreeNode, child : TreeNode, idx : number) : void {
    let self = this;
    let parent_obj : EditorManagerObject = this.get_manager_object(parent);
    let child_obj : EditorManagerObject = this.get_manager_object(child);
    assert(!child_obj.visible);
    assert(parent_obj.visible);

    child_obj.visible = true;

    let tree_id = child.get_tree().get_id();
    let child_id = child.get_id();
    let child_full_id = this.compute_full_id(child);
    let parent_full_id = this.compute_full_id(parent);

    // Add new code to DOM
    let html_code = Mustache.render(STEP_TMPL, {
      eid: tree_id,
      id: child.get_id(),
    });
    if (idx === 0) {
      $(`#${parent_full_id}_children`).prepend(html_code);
    } else {
      let prev_child_full_id = this.compute_full_id(parent.get_child(idx-1));
      $(`#${prev_child_full_id}`).after(html_code);
    }
    this.open_children(child, false);
    $(`#${child_full_id}_btn_toggle_children`).click(this.toggle_children.bind(this, child));
    this.open_data2(child, false);
    this.close_data2(child, false);
    $(`#${child_full_id}_btn_toggle_data2`).click(this.toggle_data2.bind(this, child));
    $(`#${child_full_id}_btn_close_all_children`).click(function() {
      self.open_children(child);
      for (let child2 of child.get_children()) {
        self.close_children(child2);
      }
    });
    this.painter.paint_node(child);

    // Recur on children
    for (let [idx2, child2] of child.get_children().entries()) {
      this.make_subtree_visible(child, child2, idx2);
    }
  }

  make_subtree_hidden(node : TreeNode) : void {
    let obj : EditorManagerObject = this.get_manager_object(node);
    assert(obj.visible);
    obj.visible = false;
    for (let child of node.children) {
      this.make_subtree_hidden(child);
    }
  }
}

const STEP_ROOT_TMPL = `
<div id="{{ eid }}_step_{{ id }}" class="step">
  <div id="{{ eid }}_step_{{ id }}_children"></div>
</div>
`;

const STEP_TMPL = `
<div id="{{ eid }}_step_{{ id }}" class="step">
  <div id="{{ eid }}_step_{{ id }}_row" class="step_row">
    <div id="{{ eid }}_step_{{ id }}_handle" class="step_handle">
      <button id="{{ eid }}_step_{{ id }}_btn_toggle_children" class="mini_button"></button>
      <button id="{{ eid }}_step_{{ id }}_btn_toggle_data2" class="mini_button"></button>
      <button id="{{ eid }}_step_{{ id }}_btn_close_all_children" class="mini_button"></button>
      <button id="{{ eid }}_step_{{ id }}_btn_create" class="mini_button"><object style="display: block;" data="svg/c_icon.svg" type="image/svg+xml"></object></button>
      <button id="{{ eid }}_step_{{ id }}_btn_kill" class="mini_button"><object style="display: block;" data="svg/k_icon.svg" type="image/svg+xml"></object></button>
    </div>
    <div id="{{ eid }}_step_{{ id }}_data" class="step_data">
      <div id="{{ eid }}_step_{{ id }}_data1" class="step_data1"></div>
      <div id="{{ eid }}_step_{{ id }}_data2" class="step_data2" style="display: none;"></div>
    </div>
  </div>
  <div id="{{ eid }}_step_{{ id }}_children" class="step_children"></div>
</div>
`;