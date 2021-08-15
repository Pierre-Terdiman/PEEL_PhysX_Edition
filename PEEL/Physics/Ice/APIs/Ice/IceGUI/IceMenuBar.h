///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMENUBAR_H
#define ICEMENUBAR_H

	class ICEGUI_API IceMenuBar : public IceGUIElement
	{
		public:
						IceMenuBar(IceWindow* parent);
		virtual			~IceMenuBar();

				void	AddMenu(const char* item, IceMenu* menu);
				void	SetEnabled(int id, bool b);
				void	SetChecked(int id, bool b);
				void	Modify(int id, int newId, const char* newItem);

				bool	IsEnabled(int id)	const;
				bool	IsChecked(int id)	const;
				int		GetHeight()			const;
		private:
				PREVENT_COPY(IceMenuBar);
	};

#endif	// ICEMENUBAR_H