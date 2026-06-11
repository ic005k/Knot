// 扩展安装时创建右键菜单
chrome.runtime.onInstalled.addListener(() => {
  chrome.contextMenus.removeAll(() => {
    chrome.contextMenus.create({
      id: "save_to_knot_note",
      title: "保存到 Knot 笔记",
      contexts: ["selection"]
    });
  });
});

// 右键点击回调
chrome.contextMenus.onClicked.addListener(async (info, tab) => {
  if (info.menuItemId !== "save_to_knot_note") return;

  // 获取选中富文本HTML
  let selectHtml = "";
  try {
    const res = await chrome.scripting.executeScript({
      target: { tabId: tab.id },
      func: () => {
        const sel = window.getSelection();
        if (sel.rangeCount === 0) return "";
        const frag = sel.getRangeAt(0).cloneContents();
        return new XMLSerializer().serializeToString(frag);
      }
    });
    selectHtml = res[0].result || "";
  } catch (e) {}

  // 暂存网页数据
  const noteData = {
    title: tab.title || "",
    url: tab.url || "",
    text: info.selectionText || "",
    html: selectHtml
  };
  await chrome.storage.local.set({ tempNote: noteData });

  // 打开选择笔记本弹窗
  chrome.action.openPopup();
});